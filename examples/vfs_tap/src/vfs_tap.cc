/*
 * \brief  Tap device emulation
 * \author Johannes Schlatow
 * \date   2022-01-21
 */

/*
 * Copyright (C) 2022 Genode Labs GmbH
 *
 * This file is part of the Genode OS framework, which is distributed
 * under the terms of the GNU Affero General Public License version 3.
 */

/* Genode includes */
#include <os/vfs.h>
#include <vfs/single_file_system.h>
#include <vfs/dir_file_system.h>
#include <vfs/readonly_value_file_system.h>
#include <vfs/value_file_system.h>
#include <util/xml_generator.h>
#include <vfs/single_file_system.h>
#include <nic/packet_allocator.h>
#include <nic_session/connection.h>
#include <net/mac_address.h>

namespace Vfs {
	/* overload Value_file_system to work with Net::Mac_address */
	class Mac_file_system;

	/* main file system */
	class Tap_file_system;
}


class Vfs::Mac_file_system : public Value_file_system<Net::Mac_address>
{
	public:

		Mac_file_system(Name const & name, Net::Mac_address const & mac)
		: Value_file_system(name, mac)
		{ }

		using Value_file_system<Net::Mac_address>::value;

		Net::Mac_address value()
		{
			Net::Mac_address val { };
			Net::ascii_to(buffer().string(), val);

			return val;
		}

		Net::Mac_address value() const
		{
			Net::Mac_address val { };
			Net::ascii_to(buffer().string(), val);

			return val;
		}
};


struct Vfs::Tap_file_system
{
	using Name  = String<64>;
	using Label = String<64>;

	struct Local_factory;

	struct Data_file_system;

	struct Compound_file_system;

	struct Device_update_handler;
};


/**
 * Interface for upwards reporting if the tap device state changed.
 * Currently, it is only used for triggering the info fs to read the
 * mac address from the device.
 */
struct Vfs::Tap_file_system::Device_update_handler : Interface
{
	virtual void device_state_changed() = 0;
};


/**
 * File system node for processing the packet data read/write
 */
class Vfs::Tap_file_system::Data_file_system : public Single_file_system
{
	private:

		struct Tap_vfs_handle : Single_vfs_handle
		{
			using Read_result  = File_io_service::Read_result;
			using Write_result = File_io_service::Write_result;

			using Signal_handler = Genode::Io_signal_handler<Tap_vfs_handle>;

			static constexpr size_t PKT_SIZE = Nic::Packet_allocator::DEFAULT_PACKET_SIZE;
			static constexpr size_t BUF_SIZE = Nic::Session::QUEUE_SIZE * PKT_SIZE;

			Genode::Env          &_env;
			Vfs::Env::User       &_vfs_user;
			Nic::Packet_allocator _pkt_alloc;
			Nic::Connection       _nic;
			bool                  _link_state { false };

			bool _notifying = false;
			bool _blocked   = false;

			Signal_handler _link_state_handler { _env.ep(), *this, &Tap_vfs_handle::_handle_link_state};
			Signal_handler _read_avail_handler { _env.ep(), *this, &Tap_vfs_handle::_handle_read_avail };
			Signal_handler _ack_avail_handler  { _env.ep(), *this, &Tap_vfs_handle::_handle_ack_avail };

			void _handle_ack_avail()
			{
				while (_nic.tx()->ack_avail()) {
					_nic.tx()->release_packet(_nic.tx()->get_acked_packet()); }
			}

			void _handle_read_avail()
			{
				if (!read_ready())
					return;

				if (_blocked) {
					_blocked = false;
					_vfs_user.wakeup_vfs_user();
				}

				if (_notifying) {
					_notifying = false;
					read_ready_response();
				}
			}

			void _handle_link_state()
			{
				_link_state = _nic.link_state();
				_handle_read_avail();
			}

			Tap_vfs_handle(Genode::Env            &env,
			               Vfs::Env::User         &vfs_user,
			               Allocator              &alloc,
			               Label            const &label,
			               Directory_service      &ds,
			               File_io_service        &fs,
			               int                     flags)
			: Single_vfs_handle  { ds, fs, alloc, flags },
			  _env(env),
			  _vfs_user(vfs_user),
			  _pkt_alloc(&alloc),
			  _nic(_env, &_pkt_alloc, BUF_SIZE, BUF_SIZE, label.string())
			{
				_nic.link_state_sigh(_link_state_handler);
				_link_state = _nic.link_state();
				_nic.tx_channel()->sigh_ack_avail      (_ack_avail_handler);
				_nic.rx_channel()->sigh_ready_to_ack   (_read_avail_handler);
				_nic.rx_channel()->sigh_packet_avail   (_read_avail_handler);
			}

			bool notify_read_ready() override {
				_notifying = true;
				return true;
			}

			Net::Mac_address mac_address() {
				return _nic.mac_address(); }

			/************************
			 * Vfs_handle interface *
			 ************************/

			bool read_ready() const override
			{
				auto &nonconst_this = const_cast<Tap_vfs_handle &>(*this);
				auto &rx = *nonconst_this._nic.rx();

				return _link_state && rx.packet_avail() && rx.ready_to_ack();
			}

			bool write_ready() const override
			{
				/* wakeup from WRITE_ERR_WOULD_BLOCK not supported */
				return _link_state;
			}

			Read_result read(Byte_range_ptr const &dst, size_t &out_count) override
			{
				if (!read_ready()) {
					_blocked = true;
					return Read_result::READ_QUEUED;
				}

				out_count = 0;

				/* process a single packet from rx stream */
				Genode::Packet_descriptor const rx_pkt { _nic.rx()->get_packet() };

				if (rx_pkt.size() > 0 &&
					 _nic.rx()->packet_valid(rx_pkt)) {

					const char *const rx_pkt_base {
						_nic.rx()->packet_content(rx_pkt) };

					out_count = min(rx_pkt.size(), dst.num_bytes);
					memcpy(dst.start, rx_pkt_base, out_count);

					_nic.rx()->acknowledge_packet(rx_pkt);
				}

				return Read_result::READ_OK;
			}

			Write_result write(Const_byte_range_ptr const &src, size_t &out_count) override
			{
				out_count = 0;

				_handle_ack_avail();

				if (!_nic.tx()->ready_to_submit()) {
					return Write_result::WRITE_ERR_WOULD_BLOCK;
				}
				try {
					Genode::Packet_descriptor tx_pkt {
						_nic.tx()->alloc_packet(src.num_bytes) };

					void *tx_pkt_base {
						_nic.tx()->packet_content(tx_pkt) };

					memcpy(tx_pkt_base, src.start, src.num_bytes);

					_nic.tx()->submit_packet(tx_pkt);
					out_count = src.num_bytes;

					return Write_result::WRITE_OK;
				} catch (...) {

					Genode::warning("exception while trying to forward packet from "
					                "driver to Nic connection TX");

					return Write_result::WRITE_ERR_INVALID;
				}
			}
		};

		using Registered_handle = Genode::Registered<Tap_vfs_handle>;
		using Handle_registry   = Genode::Registry<Registered_handle>;
		using Open_result       = Directory_service::Open_result;

		Label            const &_label;
		Genode::Env            &_env;
		Vfs::Env::User         &_vfs_user;
		Device_update_handler  &_device_update_handler;
		Handle_registry         _handle_registry { };

	public:

		Data_file_system(Genode::Env            & env,
		                 Vfs::Env::User         & vfs_user,
		                 Name             const & name,
		                 Label            const & label,
		                 Device_update_handler  & handler)
		:
			Single_file_system(Node_type::TRANSACTIONAL_FILE, name.string(),
			                   Node_rwx::rw(), Genode::Xml_node("<data/>")),
			_label(label), _env(env),
			_vfs_user(vfs_user), _device_update_handler(handler)
		{ }

		/* must only be called if handle has been opened */
		Tap_vfs_handle &device()
		{
			Tap_vfs_handle *dev = nullptr;
			_handle_registry.for_each([&] (Tap_vfs_handle &handle) {
				dev = &handle;
			 });

			struct Device_unavailable { };

			if (!dev)
				throw Device_unavailable();

			return *dev;
		}

		static const char *name()   { return "data"; }
		char const *type() override { return "data"; }


		/*********************************
		 ** Directory service interface **
		 *********************************/

		Open_result open(char const  *path, unsigned flags,
		                 Vfs::Vfs_handle **out_handle,
		                 Allocator   &alloc) override
		{
			if (!_single_file(path))
				return Open_result::OPEN_ERR_UNACCESSIBLE;

			/* A tap device is exclusive open, thus return error if already opened. */
			unsigned handles = 0;
			_handle_registry.for_each([&handles] (Tap_vfs_handle const &) {
				handles++;
			});
			if (handles) return Open_result::OPEN_ERR_EXISTS;

			try {
				*out_handle = new (alloc)
					Registered_handle(_handle_registry, _env, _vfs_user, alloc,
					                  _label.string(), *this, *this, flags);
				_device_update_handler.device_state_changed();
				return Open_result::OPEN_OK;
			}
			catch (Genode::Out_of_ram)  { return Open_result::OPEN_ERR_OUT_OF_RAM; }
			catch (Genode::Out_of_caps) { return Open_result::OPEN_ERR_OUT_OF_CAPS; }
		}

};


struct Vfs::Tap_file_system::Local_factory : File_system_factory,
                                             Device_update_handler
{
	using Mac_addr_fs = Mac_file_system;

	Name             const _name;
	Label            const _label;
	Net::Mac_address const _default_mac { };
	Vfs::Env              &_env;

	Data_file_system   _data_fs { _env.env(), _env.user(), _name, _label, *this };

	struct Info
	{
		Name        const &_name;
		Mac_addr_fs const &_mac_addr_fs;

		Info(Name        const & name,
		     Mac_addr_fs const & mac_addr_fs)
		: _name(name),
		  _mac_addr_fs(mac_addr_fs)
		{ }

		void print(Genode::Output &out) const
		{

			char buf[128] { };
			Genode::Xml_generator xml(buf, sizeof(buf), "tap", [&] () {
				xml.attribute("mac_addr", String<20>(_mac_addr_fs.value()));
				xml.attribute("name", _name);
			});
			Genode::print(out, Genode::Cstring(buf));
		}
	};

	Mac_addr_fs                          _mac_addr_fs   { "mac_addr", _default_mac };

	Info                                 _info          { _name, _mac_addr_fs };
	Readonly_value_file_system<Info>     _info_fs       { "info",       _info };

	/*****************************
	 ** Device update interface **
	 *****************************/

	void device_state_changed() override
	{
		/* update mac address */
		_mac_addr_fs.value(_data_fs.device().mac_address());

		/* propagate changes to info_fs */
		_info_fs.value(_info);
	}

	/***********************
	 ** Factory interface **
	 ***********************/

	Vfs::File_system *create(Vfs::Env&, Xml_node node) override
	{
		if (node.has_type("data"))      return &_data_fs;
		if (node.has_type("info"))      return &_info_fs;

		return nullptr;
	}

	/***********************
	 ** Constructor, etc. **
	 ***********************/

	static Name name(Xml_node config)
	{
		return config.attribute_value("name", Name("tap"));
	}

	Local_factory(Vfs::Env &env, Xml_node config)
	:
		_name       (name(config)),
		_label      (config.attribute_value("label", Label(""))),
		_env(env)
	{ }
};


class Vfs::Tap_file_system::Compound_file_system : private Local_factory,
                                                   public  Vfs::Dir_file_system
{
	private:

		typedef Tap_file_system::Name Name;

		typedef String<200> Config;
		static Config _config(Name const &name)
		{
			char buf[Config::capacity()] { };

			/*
			 * By not using the node type "dir", we operate the
			 * 'Dir_file_system' in root mode, allowing multiple sibling nodes
			 * to be present at the mount point.
			 */
			Genode::Xml_generator xml(buf, sizeof(buf), "compound", [&] () {

				xml.node("data", [&] () {
					xml.attribute("name", name); });

				xml.node("dir", [&] () {
					xml.attribute("name", Name(".", name));
					xml.node("info",       [&] () {});
				});
			});

			return Config(Genode::Cstring(buf));
		}

	public:

		Compound_file_system(Vfs::Env &vfs_env, Genode::Xml_node node)
		:
			Local_factory(vfs_env, node),
			Vfs::Dir_file_system(vfs_env,
			                     Xml_node(_config(Local_factory::name(node)).string()),
			                     *this)
		{ }

		static const char *name() { return "tap"; }

		char const *type() override { return name(); }
};


extern "C" Vfs::File_system_factory *vfs_file_system_factory(void)
{
	struct Factory : Vfs::File_system_factory
	{
		Vfs::File_system *create(Vfs::Env &env, Genode::Xml_node config) override
		{
			return new (env.alloc())
				Vfs::Tap_file_system::Compound_file_system(env, config);
		}
	};

	static Factory f;
	return &f;
}
