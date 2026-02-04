# Configuration Reference

---

## Configuration Structure and Adapter Groups

EKiosk settings are organized into logical groups ("adapters") that correspond to different aspects of the system. Each configuration file is loaded into a specific group, and keys are namespaced accordingly. The main adapter groups are:

- **TerminalSettings** (`AdapterNames::TerminalAdapter`): Terminal-wide and system settings (e.g., system.ini, terminal.ini)
- **DealerSettings** (`AdapterNames::DealerAdapter`): Dealer/provider-specific settings (e.g., commissions.xml)
- **UserSettings** (`AdapterNames::UserAdapter`): User-specific overrides (e.g., user.ini)
- **Directory** (`AdapterNames::Directory`): Directory and capacity data (e.g., directory.xml)
- **Extensions** (`AdapterNames::Extensions`): Extensions and plugin settings (e.g., extensions.xml)

When loading, each file is assigned to an adapter group. Keys from files loaded later (e.g., user.ini in UserSettings) can override keys from earlier files in the same group (e.g., system.ini in TerminalSettings). This enables a layered configuration model:

- **System-wide defaults** (system.ini, TerminalSettings)
- **Dealer/provider settings** (DealerSettings)
- **User/terminal-specific overrides** (user.ini, UserSettings)

**Example:**

If both system.ini and user.ini define the same key under TerminalSettings, the value from user.ini will be used, as it is loaded later and has higher precedence.

---

---

## apps/EKiosk

**Config file:** `PaymentProcessor.ini` (location: build/<preset>/apps/kiosk/ or user-specified)

### [common]

- `user_data_path`: Path for user data (default: user)
- `plugins_path`: Path to plugins directory
- `content_path`: Path to content resources
- `interface_path`: Path to UI interface
- `configuration`: Name of configuration profile (e.g., terminal_ru)
- `working_directory`: Working directory for the app (should point to a directory with all required files for full operation)
- `standalone`: true/false, run in standalone mode

### [runtime]

- `reboot_count`: Number of reboots before action (int, default: 99)
- `check_balance_sim`, `check_number_sim`: Enable SIM balance/number check (bool)
- `vpn_point`: VPN connection name (string)
- `ussd_balance_sim`, `ussd_number_sim`: USSD codes for SIM queries (string)
- `index_check_balance`: Index for balance check (int)
- `show_print_dialog`: Show print dialog (bool)
- `chek_width`, `chek_left_size`, `chek_small_text`, `printing_chek`, `chek_small_beetwen_string`: Printer formatting options (int/bool)
- `ras_error_interval_reboot`: Reboot interval on error (int)
- `default_lang`: Default language (string)
- `search_validator`, `search_coin_acceptor`, `search_printer`, `search_modem`, `search_watchdog`: Enable device search (bool)
- `prt_win_width`, `prt_win_height`, `prt_win_font_size`, `prt_win_left_margin`, `prt_win_right_margin`, `prt_win_top_margin`, `prt_win_bottom_margin`: Windows printer formatting (int)
- `exist_counter_printer_chek`, `exist_counter_chek`: Enable counters (bool)
- `counter_len_rulon`, `counter_len_chek`, `counter_ring_value`: Counter values (int/double)
- `sms_send_number`: SMS number for notifications (string)
- `sms_err_validator`, `sms_err_printer`, `sms_err_balance_agent`, `sms_err_sim_balance`, `sms_err_lock_terminal`, `sms_err_connection`: SMS error notifications (bool)
- `sms_value_balance_agent`: Balance threshold for SMS (double)
- `status_validator_jam_in_box`, `status_validator_jam_in_box_value_counter`, `status_validator_jam_in_box_lockers`: Validator jam settings (bool/int)
- `lock_duplicate_nominal`: Lock duplicate nominal (bool)
- `auto_update_status`: Enable auto-update status (bool)
- `tpl`: Template (string, e.g., "tjk")
- `test`, `inspect`: Test/inspect mode (bool)

**Example:**

```ini
[common]
user_data_path=user
plugins_path=bin/plugins
content_path=interface
interface_path=interface/touch17
configuration=terminal_ru
working_directory=MyWorkDir
standalone=true

[runtime]
reboot_count=99
check_balance_sim=true
vpn_point=MyVPN
default_lang=ru
test=false
```

---

## apps/Updater

**Config file:** `updater.ini`, `bits.ini`

- `working_dir`: Updater working directory (string)
- `config_url`: URL for config updates (string)
- `update_url`: URL for update packages (string)
- `version`: Current version (string)
- `app_id`: Application ID (string)
- `configuration`: Configuration profile (string)
- `ap`: Additional parameter (string)
- `arg`: Command-line argument (string)
- `bits/ignore`: Ignore BITS (bool)
- `bits/priority`: BITS priority (int)
- `directory/ignore`: List of directories to ignore (string list)
- `component/optional`: List of optional components (string list)
- `validator/required_files`: List of required files (string list)

---

## apps/WatchService

**Config file:** `user.ini`, `watchservice.ini`

See [WatchService Application Documentation](../apps/watchservice.md) for complete configuration details.

- `watchdog/taboo_enabled`: Enable forbidden app monitoring (bool)
- `applications`: List of forbidden applications (string list)
- `check_timeout`: Timeout for forbidden app check (int, ms)

---

## modules/Common/Application

**Config file:** `user.ini`

- `log/level`: Log verbosity level (int)

---

## modules/Connection

**Config file:** `connection.ini`

- `check_period`: Interval for status checks (ms)
- `ping_period`: Interval for pings (ms)
- `ping_timeout`: Timeout for ping requests (ms)
- `check_host`: Host for connection checks
- `check_response`: Expected response string

---

## modules/NetworkTaskManager

**Config file:** `network_task_manager.ini`

- `max_tasks`: Maximum number of concurrent network tasks
- `timeout`: Default network task timeout (ms)

---

## modules/UpdateEngine

**Config file:** `updater.ini`, `bits.ini`

- See apps/Updater above for keys (shared)

---

## modules/SettingsManager

**Config file:** (varies)

- Handles arbitrary keys for settings import/export. See module docs for details.

---

## modules/SDK/Plugins

**Config file:** (per-plugin, see plugin docs)

- Each plugin may define its own config keys. See [docs/plugins/](plugins/) and each plugin's README.md for details.

---

## modules/common/log

**Config file:** `<appname>.ini` (e.g., PaymentProcessor.ini)

- `common/working_directory`: Used to determine log file location. If not set, uses executable directory.

---

## plugins

Each plugin may define its own configuration file and keys. See [docs/plugins/](plugins/) and the plugin's README.md for a full list and descriptions.

---

## General Notes

- All config files are standard INI format.
- Paths can be absolute or relative to the working directory.
- Each module/plugin may introduce its own config file and keys—see their documentation for details.
- For environment variable overrides, see [docs/getting-started.md](getting-started.md) and [docs/build-guide.md](build-guide.md).

---

## How to Update

- When adding a new config key, update this file and the relevant module/plugin documentation.
- Provide example .ini snippets for new features.
- Keep descriptions clear and concise.

---

_Last updated: January 2026_

## plugins/AdBackend

**Config file:** `adbackend.ini`

### [ad]

- `server_url`: URL for ad server
- `refresh_interval`: Interval for ad refresh (ms)

---

## General Notes

- All config files are standard INI format.
- Paths can be absolute or relative to the working directory.
- Each module/plugin may introduce its own config file and keys—see their documentation for details.
- For environment variable overrides, see [docs/getting-started.md](getting-started.md) and [docs/build-guide.md](build-guide.md).

---

## How to Update

- When adding a new config key, update this file and the relevant module/plugin documentation.
- Provide example .ini snippets for new features.
- Keep descriptions clear and concise.
