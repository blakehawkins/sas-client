/**
 * @file sas.h Definition of SAS class used for reporting events and markers
 * to Service Assurance Server
 *
 * Service Assurance Server client library
 * Copyright (C) 2013  Metaswitch Networks Ltd
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version, along with the "Special Exception" for use of
 * the program along with SSL, set forth below. This program is distributed
 * in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details. You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * The author can be reached by email at clearwater@metaswitch.com or by
 * post at Metaswitch Networks Ltd, 100 Church St, Enfield EN2 6BQ, UK
 *
 * Special Exception
 * Metaswitch Networks Ltd  grants you permission to copy, modify,
 * propagate, and distribute a work formed by combining OpenSSL with The
 * Software, or a work derivative of such a combination, even if such
 * copying, modification, propagation, or distribution would otherwise
 * violate the terms of the GPL. You must comply with the GPL in all
 * respects for all of the code used other than OpenSSL.
 * "OpenSSL" means OpenSSL toolkit software distributed by the OpenSSL
 * Project and licensed under the OpenSSL Licenses, or a work based on such
 * software and licensed under the OpenSSL Licenses.
 * "OpenSSL Licenses" means the OpenSSL License and Original SSLeay License
 * under which the OpenSSL Project distributes the OpenSSL toolkit software,
 * as those licenses appear in the file LICENSE-OPENSSL.
 */

#ifndef SAS_H__
#define SAS_H__

// This is autogenerated by the sas-client configure script.
#include <config.h>

#include <stdint.h>
#include <string.h>
#include <string>

#if HAVE_ATOMIC
  #include <atomic>
#elif HAVE_CSTDATOMIC
  #include <cstdatomic>
#else
  #error "Atomic types not supported"
#endif

#include "eventq.h"

// Marker IDs
static const int MARKER_ID_START = 0x01000003;
static const int MARKER_ID_END = 0x01000004;
static const int MARKER_ID_DAILED_DIGITS = 0x01000005;
static const int MARKER_ID_CALLING_DN = 0x01000006;
static const int MARKER_ID_CALLED_DN = 0x01000007;
static const int MARKER_ID_SIP_REGISTRATION = 0x010B0004;
static const int MARKER_ID_SIP_ALL_REGISTER = 0x010B0005;
static const int MARKER_ID_SIP_CALL_ID = 0x010C0001;
static const int MARKER_ID_IMS_CHARGING_ID = 0x010C0002;
static const int MARKER_ID_VIA_BRANCH_PARAM = 0x010C0003;

static const int MARKER_ID_OUTBOUND_CALLING_URI = 0x05000003;
static const int MARKER_ID_INBOUND_CALLING_URI = 0x05000004;
static const int MARKER_ID_OUTBOUND_CALLED_URI = 0x05000005;
static const int MARKER_ID_INBOUND_CALLED_URI = 0x05000006;

static const int MARKER_ID_PROTOCOL_ERROR = 0x01000001;

// SAS::init return codes
static const int SAS_INIT_RC_OK = 0;
static const int SAS_INIT_RC_ERR = 1;

class SAS
{
public:
  typedef uint64_t TrailId;

  class Message
  {
  public:
    static const int MAX_NUM_STATIC_PARAMS = 20;
    static const int MAX_NUM_VAR_PARAMS = 20;

    inline Message(TrailId trail,
                   uint32_t id,
                   uint32_t instance) :
      _params_buffer(),
      _trail(trail),
      _id(id),
      _instance(instance),
      _num_static_data(0),
      _num_var_data(0)
    {
      // Write the length of the static data into the params buffer (0 for a
      // new Message).
      write_int16(_params_buffer, 0);
    }

    virtual ~Message()
    {
    }

    Message& add_static_param(uint32_t param);

    Message& add_var_param(size_t len, uint8_t* data);

    inline Message& add_var_param(size_t len, char* s)
    {
      return add_var_param(len, (uint8_t*)s);
    }

    inline Message& add_var_param(const char* s)
    {
      return add_var_param(strlen(s), (uint8_t*)s);
    }

    inline Message& add_var_param(const std::string& s)
    {
      return add_var_param(s.length(), (uint8_t*)s.data());
    }

    friend class SAS;

  protected:
    std::string _params_buffer;

  private:
    TrailId _trail;
    uint32_t _id;
    uint32_t _instance;

    uint32_t _num_static_data;
    uint32_t _num_var_data;
    uint32_t _var_data_lengths[MAX_NUM_VAR_PARAMS];
  };

  class Event : public Message
  {
  public:
    // Event IDs as defined by the application are restricted to 24 bits.
    // This is because the top byte of the event ID is reserved and set to 0x0F.
    // It is comprised of:
    //   - The top nibble, which is reserved for future use and must be set to
    //     0x0.
    //   - the bottom nibble, which SAS requires be set to the value 0xF.
    inline Event(TrailId trail, uint32_t event, uint32_t instance) :
      Message(trail,
              ((event & 0x00FFFFFF) | 0x0F000000),
              instance)
    {
    }

    std::string to_string();
  };

  class Marker : public Message
  {
  public:
    inline Marker(TrailId trail, uint32_t marker, uint32_t instance) :
      Message(trail, marker, instance)
    {
    }

    enum Scope
    {
      None = 0,
      Branch = 1,
      Trace = 2
    };

    std::string to_string(Scope scope);
  };

  enum log_level_t {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARNING = 1,
    LOG_LEVEL_STATUS = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_VERBOSE = 4,
    LOG_LEVEL_DEBUG = 5,
  };

  typedef void (sas_log_callback_t)(log_level_t level,
                                    const char *module,
                                    int line_number,
                                    const char *fmt,
                                    ...);

  // A simple implementation of sas_log_callback_t that logs messages to stdout.
  static void log_to_stdout(log_level_t level,
                            const char *module,
                            int line_number,
                            const char *fmt,
                            ...);

  // A simple implementation of sas_log_callback_t that discards all logs.
  static void discard_logs(log_level_t level,
                           const char *module,
                           int line_number,
                           const char *fmt,
                           ...);

  static int init(const std::string& system_name,
                   const std::string& system_type,
                   const std::string& resource_identifier,
                   const std::string& sas_address,
                   sas_log_callback_t* log_callback);
  static void term();
  static TrailId new_trail(uint32_t instance);
  static void report_event(Event& event);
  static void report_marker(Marker& marker, Marker::Scope scope=Marker::Scope::None);

private:
  class Connection
  {
  public:
    Connection(const std::string& system_name,
               const std::string& system_type,
               const std::string& resource_identifier,
               const std::string& sas_address);
    ~Connection();

    void send_msg(std::string msg);

    static void* writer_thread(void* p);

  private:
    bool connect_init();
    void writer();

    std::string _system_name;
    std::string _system_type;
    std::string _resource_identifier;
    std::string _sas_address;

    eventq<std::string> _msg_q;

    pthread_t _writer;

    // Socket for the connection.
    int _sock;

    /// Send timeout for the socket in seconds.
    static const int SEND_TIMEOUT = 30;

    /// Maximum depth of SAS message queue.
    static const int MAX_MSG_QUEUE = 1000;
  };

  static void write_hdr(std::string& s, uint16_t msg_length, uint8_t msg_type);
  static void write_int8(std::string& s, uint8_t c);
  static void write_int16(std::string& s, uint16_t v);
  static void write_int32(std::string& s, uint32_t v);
  static void write_int64(std::string& s, uint64_t v);
  static void write_data(std::string& s, size_t length, const char* data);
  static void write_timestamp(std::string& s);
  static void write_trail(std::string& s, TrailId trail);

  static std::atomic<TrailId> _next_trail_id;
  static Connection* _connection;
  static sas_log_callback_t* _log_callback;
};

#endif
