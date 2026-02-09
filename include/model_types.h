/**
 * @file model.h
 * @brief 打印机管理数据模型（ORM 方式）
 * 
 * 支持打印机配置、打印任务管理等功能
 */

#ifndef RESTFUL_MODEL_H
#define RESTFUL_MODEL_H

#include "model_database.h"
#include <string>
#include <ctime>

// 安全的整数转换函数
template<typename T>
T safeSto(const std::string& str, T default_val = 0) {
    if (str.empty()) return default_val;
    try {
        return static_cast<T>(std::stoll(str));
    } catch (const std::exception&) {
        return default_val;
    }
}

// 获取当前时间戳
std::string getCurrentTimestamp() {
    return std::to_string(std::time(nullptr));
}

// 用户模型（增强版 - 支持3A功能）
class UserModel : public Model {
public:
    UserModel(Database* db) : Model(db, "users") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_TEXT("username"),
            DB_FIELD_TEXT("password"),
            DB_FIELD_TEXT("role"),
            DB_FIELD_INTEGER("status"),
            DB_FIELD_TEXT("email"),
            DB_FIELD_TEXT("department"),
            DB_FIELD_TEXT("phone"),
            DB_FIELD_TEXT("created_at"),
            DB_FIELD_TEXT("last_login"),
            DB_FIELD_INTEGER("failed_attempts"),
            DB_FIELD_TEXT("locked_until"),
            DB_FIELD_TEXT("password_changed_at"),
            DB_FIELD_INTEGER("password_history")
        };
    }
    
    // 创建用户（增强版）
    bool create(const std::string& username, const std::string& password, const std::string& role = "user",
                const std::string& email = "", const std::string& department = "") {
        RowData data;
        data["username"] = username;
        data["password"] = password;  // 实际应用中应该使用bcrypt等加密
        data["role"] = role;
        data["status"] = "1";
        data["email"] = email;
        data["department"] = department;
        data["phone"] = "";
        data["created_at"] = "";
        data["last_login"] = "";
        data["failed_attempts"] = "0";
        data["locked_until"] = "";
        data["password_changed_at"] = "";
        data["password_history"] = "0";
        return insert(data);
    }
    
    // 根据用户名查找
    RowData findByUsername(const std::string& username) {
        return query().where("username = '" + db_->escapeString(username) + "'").first();
    }
    
    // 根据ID查找
    RowData findById(const std::string& id) {
        return query().where("id = " + id).first();
    }
    
    // 验证用户（增强版 - 记录登录失败）
    bool authenticate(const std::string& username, const std::string& password, std::string& error_msg) {
        RowData user = findByUsername(username);
        if (user.empty()) {
            error_msg = "User not found";
            return false;
        }
        
        // 检查账户状态
        if (user["status"] != "1") {
            error_msg = "Account is disabled";
            return false;
        }
        
        // 检查账户锁定
        if (!user["locked_until"].empty()) {
            error_msg = "Account is locked due to too many failed attempts";
            return false;
        }
        
        // 检查密码
        if (user["password"] != password) {
            // 增加失败次数
            int failed_attempts = safeSto<int>(user["failed_attempts"], 0) + 1;
            RowData update_data;
            update_data["failed_attempts"] = std::to_string(failed_attempts);
            
            // 如果失败次数达到5次，锁定账户1小时
            if (failed_attempts >= 5) {
                update_data["locked_until"] = std::to_string(std::time(nullptr) + 3600);
                error_msg = "Account locked due to too many failed attempts";
            } else {
                error_msg = "Invalid password. Attempts remaining: " + std::to_string(5 - failed_attempts);
            }
            
            updateById(safeSto<int64_t>(user["id"], 0), update_data);
            return false;
        }
        
        // 登录成功，重置失败次数
        RowData update_data;
        update_data["failed_attempts"] = "0";
        update_data["last_login"] = getCurrentTimestamp();
        updateById(safeSto<int64_t>(user["id"], 0), update_data);
        
        return true;
    }
    
    // 简化版验证（向后兼容）
    bool authenticate(const std::string& username, const std::string& password) {
        std::string error_msg;
        return authenticate(username, password, error_msg);
    }
    
    // 检查权限
    bool hasPermission(const std::string& username, const std::string& permission) {
        RowData user = findByUsername(username);
        if (user.empty()) return false;
        
        std::string role = user["role"];
        
        // 管理员拥有所有权限
        if (role == "admin") return true;
        
        // 运营员拥有部分权限
        if (role == "operator") {
            return permission == "read" || permission == "update" || permission == "create";
        }
        
        // 普通用户只有读权限
        if (role == "user") {
            return permission == "read";
        }
        
        return false;
    }
    
    // 检查是否为管理员
    bool isAdmin(const std::string& username) {
        RowData user = findByUsername(username);
        if (user.empty()) return false;
        return user["role"] == "admin";
    }
    
    // 获取所有活跃用户
    ResultSet activeUsers() {
        return query().where("status = 1").get();
    }
    
    // 按部门查询用户
    ResultSet findByDepartment(const std::string& department) {
        return query().where("department = '" + db_->escapeString(department) + "'").get();
    }
    
    // 更新用户状态
    bool updateStatus(const std::string& id, const std::string& status) {
        RowData data;
        data["status"] = status;
        return updateById(std::stoll(id), data);
    }
    
    // 修改密码
    bool changePassword(const std::string& username, const std::string& old_password, const std::string& new_password) {
        RowData user = findByUsername(username);
        if (user.empty()) return false;
        
        if (user["password"] != old_password) return false;
        
        RowData data;
        data["password"] = new_password;
        data["password_changed_at"] = "";
        int history_count = std::stoi(user["password_history"]) + 1;
        data["password_history"] = std::to_string(history_count);
        
        return updateById(std::stoll(user["id"]), data);
    }
    
    // 锁定用户
    bool lockUser(const std::string& id) {
        RowData data;
        data["status"] = "0";
        data["locked_until"] = "";
        return updateById(std::stoll(id), data);
    }
    
    // 解锁用户
    bool unlockUser(const std::string& id) {
        RowData data;
        data["status"] = "1";
        data["failed_attempts"] = "0";
        data["locked_until"] = "";
        return updateById(std::stoll(id), data);
    }
    
    // 删除用户
    bool removeUser(const std::string& id) {
        return remove("id = " + id);
    }
};

// 路由器模型
class RouterModel : public Model {
public:
    RouterModel(Database* db) : Model(db, "routers") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_TEXT("name"),
            DB_FIELD_TEXT("ip_address"),
            DB_FIELD_TEXT("mac_address"),
            DB_FIELD_TEXT("model"),
            DB_FIELD_TEXT("location"),
            DB_FIELD_INTEGER("status"),
            DB_FIELD_INTEGER("owner_id")
        };
    }
    
    // 创建路由器
    bool create(const std::string& name, const std::string& ip_address, 
               const std::string& mac_address, const std::string& model = "",
               const std::string& location = "", int64_t owner_id = 0) {
        RowData data;
        data["name"] = name;
        data["ip_address"] = ip_address;
        data["mac_address"] = mac_address;
        data["model"] = model;
        data["location"] = location;
        data["status"] = "1";
        if (owner_id > 0) {
            data["owner_id"] = std::to_string(owner_id);
        }
        return insert(data);
    }
    
    // 根据用户ID查找路由器
    ResultSet findByOwner(int64_t owner_id) {
        return query().where("owner_id = " + std::to_string(owner_id)).get();
    }
    
    // 获取在线路由器
    ResultSet onlineRouters() {
        return query().where("status = 1").get();
    }
    
    // 更新路由器状态
    bool updateStatus(int64_t router_id, int status) {
        RowData data;
        data["status"] = std::to_string(status);
        return updateById(router_id, data);
    }
};

// 路由器设置模型（WAN设置）
class RouterWanSettingsModel : public Model {
public:
    RouterWanSettingsModel(Database* db) : Model(db, "router_wan_settings") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_INTEGER("router_id"),
            DB_FIELD_TEXT("connection_type"),
            DB_FIELD_TEXT("ip_address"),
            DB_FIELD_TEXT("subnet_mask"),
            DB_FIELD_TEXT("gateway"),
            DB_FIELD_TEXT("dns1"),
            DB_FIELD_TEXT("dns2"),
            DB_FIELD_TEXT("pppoe_username"),
            DB_FIELD_TEXT("pppoe_password"),
            DB_FIELD_INTEGER("mtu")
        };
    }
    
    // 创建WAN设置
    bool create(int64_t router_id, const std::string& connection_type) {
        RowData data;
        data["router_id"] = std::to_string(router_id);
        data["connection_type"] = connection_type;
        data["mtu"] = "1500";
        return insert(data);
    }
    
    // 根据路由器ID查找设置
    RowData findByRouterId(int64_t router_id) {
        return query().where("router_id = " + std::to_string(router_id)).first();
    }
    
    // 更新设置
    bool updateSettings(int64_t router_id, const RowData& settings) {
        RowData existing = findByRouterId(router_id);
        if (existing.empty()) {
            RowData new_settings = settings;
            new_settings["router_id"] = std::to_string(router_id);
            return insert(new_settings);
        }
        return updateById(router_id, settings);
    }
};

// 路由器WLAN设置模型
class RouterWlanSettingsModel : public Model {
public:
    RouterWlanSettingsModel(Database* db) : Model(db, "router_wlan_settings") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_INTEGER("router_id"),
            DB_FIELD_TEXT("ssid"),
            DB_FIELD_TEXT("password"),
            DB_FIELD_TEXT("encryption_type"),
            DB_FIELD_INTEGER("channel"),
            DB_FIELD_TEXT("band"),
            DB_FIELD_INTEGER("hidden"),
            DB_FIELD_INTEGER("max_connections")
        };
    }
    
    // 创建WLAN设置
    bool create(int64_t router_id, const std::string& ssid, const std::string& encryption_type) {
        RowData data;
        data["router_id"] = std::to_string(router_id);
        data["ssid"] = ssid;
        data["encryption_type"] = encryption_type;
        data["channel"] = "1";
        data["band"] = "2.4g";
        data["hidden"] = "0";
        data["max_connections"] = "32";
        return insert(data);
    }
    
    // 根据路由器ID查找设置
    RowData findByRouterId(int64_t router_id) {
        return query().where("router_id = " + std::to_string(router_id)).first();
    }
    
    // 更新设置
    bool updateSettings(int64_t router_id, const RowData& settings) {
        RowData existing = findByRouterId(router_id);
        if (existing.empty()) {
            RowData new_settings = settings;
            new_settings["router_id"] = std::to_string(router_id);
            return insert(new_settings);
        }
        return updateById(router_id, settings);
    }
};

// 路由器端口转发设置模型
class RouterPortForwardModel : public Model {
public:
    RouterPortForwardModel(Database* db) : Model(db, "router_port_forward") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_INTEGER("router_id"),
            DB_FIELD_TEXT("name"),
            DB_FIELD_INTEGER("external_port"),
            DB_FIELD_INTEGER("internal_port"),
            DB_FIELD_TEXT("protocol"),
            DB_FIELD_TEXT("internal_ip"),
            DB_FIELD_INTEGER("enabled")
        };
    }
    
    // 添加端口转发规则
    bool addRule(int64_t router_id, const std::string& name, int external_port, 
                int internal_port, const std::string& protocol, const std::string& internal_ip) {
        RowData data;
        data["router_id"] = std::to_string(router_id);
        data["name"] = name;
        data["external_port"] = std::to_string(external_port);
        data["internal_port"] = std::to_string(internal_port);
        data["protocol"] = protocol;
        data["internal_ip"] = internal_ip;
        data["enabled"] = "1";
        return insert(data);
    }
    
    // 根据路由器ID查找规则
    ResultSet findByRouterId(int64_t router_id) {
        return query().where("router_id = " + std::to_string(router_id)).get();
    }
    
    // 获取启用的规则
    ResultSet findEnabled(int64_t router_id) {
        return query().where("router_id = " + std::to_string(router_id) + " AND enabled = 1").get();
    }
};

// 路由器防火墙设置模型
class RouterFirewallModel : public Model {
public:
    RouterFirewallModel(Database* db) : Model(db, "router_firewall_rules") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_INTEGER("router_id"),
            DB_FIELD_TEXT("name"),
            DB_FIELD_TEXT("action"),
            DB_FIELD_TEXT("direction"),
            DB_FIELD_TEXT("protocol"),
            DB_FIELD_TEXT("source_ip"),
            DB_FIELD_TEXT("source_port"),
            DB_FIELD_TEXT("dest_ip"),
            DB_FIELD_TEXT("dest_port"),
            DB_FIELD_INTEGER("enabled"),
            DB_FIELD_INTEGER("priority")
        };
    }
    
    // 添加防火墙规则
    bool addRule(int64_t router_id, const std::string& name, const std::string& action, 
                const std::string& direction) {
        RowData data;
        data["router_id"] = std::to_string(router_id);
        data["name"] = name;
        data["action"] = action;
        data["direction"] = direction;
        data["enabled"] = "1";
        data["priority"] = "100";
        return insert(data);
    }
    
    // 根据路由器ID查找规则（按优先级排序）
    ResultSet findByRouterId(int64_t router_id) {
        return query().where("router_id = " + std::to_string(router_id))
                   .orderBy("priority", true)
                   .get();
    }
};

// 路由器SMTP设置模型
class RouterSmtpSettingsModel : public Model {
public:
    RouterSmtpSettingsModel(Database* db) : Model(db, "router_smtp_settings") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_INTEGER("router_id"),
            DB_FIELD_TEXT("server"),
            DB_FIELD_INTEGER("port"),
            DB_FIELD_TEXT("username"),
            DB_FIELD_TEXT("password"),
            DB_FIELD_TEXT("from_address"),
            DB_FIELD_TEXT("encryption"),
            DB_FIELD_INTEGER("enabled")
        };
    }
    
    // 创建SMTP设置
    bool create(int64_t router_id, const std::string& server) {
        RowData data;
        data["router_id"] = std::to_string(router_id);
        data["server"] = server;
        data["port"] = "587";
        data["encryption"] = "tls";
        data["enabled"] = "0";
        return insert(data);
    }
    
    // 根据路由器ID查找设置
    RowData findByRouterId(int64_t router_id) {
        return query().where("router_id = " + std::to_string(router_id)).first();
    }
    
    // 更新设置
    bool updateSettings(int64_t router_id, const RowData& settings) {
        RowData existing = findByRouterId(router_id);
        if (existing.empty()) {
            RowData new_settings = settings;
            new_settings["router_id"] = std::to_string(router_id);
            return insert(new_settings);
        }
        return updateById(router_id, settings);
    }
    
    // 获取启用的SMTP设置
    ResultSet findEnabled() {
        return query().where("enabled = 1").get();
    }
};

// 网络白名单模型
class NetworkWhitelistModel : public Model {
public:
    NetworkWhitelistModel(Database* db) : Model(db, "network_whitelist") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_INTEGER("router_id"),
            DB_FIELD_TEXT("name"),
            DB_FIELD_TEXT("type"),
            DB_FIELD_TEXT("value"),
            DB_FIELD_TEXT("description"),
            DB_FIELD_INTEGER("enabled")
        };
    }
    
    // 添加白名单规则
    bool addRule(int64_t router_id, const std::string& name, const std::string& type, 
                const std::string& value, const std::string& description = "") {
        RowData data;
        data["router_id"] = std::to_string(router_id);
        data["name"] = name;
        data["type"] = type;
        data["value"] = value;
        data["description"] = description;
        data["enabled"] = "1";
        return insert(data);
    }
    
    // 根据路由器ID查找规则
    ResultSet findByRouterId(int64_t router_id) {
        return query().where("router_id = " + std::to_string(router_id)).get();
    }
    
    // 检查是否在白名单中
    bool isWhitelisted(int64_t router_id, const std::string& value, const std::string& type = "") {
        QueryBuilder qb = query()
            .where("router_id = " + std::to_string(router_id))
            .where("value = '" + db_->escapeString(value) + "'")
            .where("enabled = 1");
        
        if (!type.empty()) {
            qb.where("type = '" + type + "'");
        }
        
        return qb.exists();
    }
};

// UPnP设置模型
class UpnpSettingsModel : public Model {
public:
    UpnpSettingsModel(Database* db) : Model(db, "upnp_settings") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_INTEGER("router_id"),
            DB_FIELD_INTEGER("enabled"),
            DB_FIELD_INTEGER("advertise_interval"),
            DB_FIELD_INTEGER("allow_remove"),
            DB_FIELD_INTEGER("secure_mode")
        };
    }
    
    // 创建UPnP设置
    bool create(int64_t router_id) {
        RowData data;
        data["router_id"] = std::to_string(router_id);
        data["enabled"] = "0";
        data["advertise_interval"] = "30";
        data["allow_remove"] = "1";
        data["secure_mode"] = "0";
        return insert(data);
    }
    
    // 根据路由器ID查找设置
    RowData findByRouterId(int64_t router_id) {
        return query().where("router_id = " + std::to_string(router_id)).first();
    }
    
    // 更新设置
    bool updateSettings(int64_t router_id, const RowData& settings) {
        RowData existing = findByRouterId(router_id);
        if (existing.empty()) {
            RowData new_settings = settings;
            new_settings["router_id"] = std::to_string(router_id);
            return insert(new_settings);
        }
        return updateById(router_id, settings);
    }
    
    // 获取启用的UPnP设置
    ResultSet findEnabled() {
        return query().where("enabled = 1").get();
    }
};

// UPnP端口映射模型
class UpnpPortMappingModel : public Model {
public:
    UpnpPortMappingModel(Database* db) : Model(db, "upnp_port_mappings") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_INTEGER("router_id"),
            DB_FIELD_TEXT("protocol"),
            DB_FIELD_INTEGER("external_port"),
            DB_FIELD_INTEGER("internal_port"),
            DB_FIELD_TEXT("internal_ip"),
            DB_FIELD_TEXT("description"),
            DB_FIELD_INTEGER("lease_duration"),
            DB_FIELD_INTEGER("enabled")
        };
    }
    
    // 添加端口映射
    bool addMapping(int64_t router_id, const std::string& protocol, int external_port, 
                  int internal_port, const std::string& internal_ip, const std::string& description = "") {
        RowData data;
        data["router_id"] = std::to_string(router_id);
        data["protocol"] = protocol;
        data["external_port"] = std::to_string(external_port);
        data["internal_port"] = std::to_string(internal_port);
        data["internal_ip"] = internal_ip;
        data["description"] = description;
        data["lease_duration"] = "3600";
        data["enabled"] = "1";
        return insert(data);
    }
    
    // 根据路由器ID查找映射
    ResultSet findByRouterId(int64_t router_id) {
        return query().where("router_id = " + std::to_string(router_id)).get();
    }
    
    // 删除映射
    bool deleteMapping(int64_t mapping_id) {
        return deleteById(mapping_id);
    }
};

// 操作日志模型（审计）
class AuditLogModel : public Model {
public:
    AuditLogModel(Database* db) : Model(db, "audit_logs") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_INTEGER("user_id"),
            DB_FIELD_TEXT("action"),
            DB_FIELD_TEXT("resource_type"),
            DB_FIELD_INTEGER("resource_id"),
            DB_FIELD_TEXT("details"),
            DB_FIELD_TEXT("ip_address")
        };
    }
    
    // 记录日志
    bool log(int64_t user_id, const std::string& action, const std::string& resource_type = "",
            int64_t resource_id = 0, const std::string& details = "", const std::string& ip_address = "") {
        RowData data;
        if (user_id > 0) {
            data["user_id"] = std::to_string(user_id);
        }
        data["action"] = action;
        data["resource_type"] = resource_type;
        data["resource_id"] = std::to_string(resource_id);
        data["details"] = details;
        data["ip_address"] = ip_address;
        return insert(data);
    }
    
    // 查询用户日志
    ResultSet findByUser(int64_t user_id) {
        return query().where("user_id = " + std::to_string(user_id))
                   .orderBy("created_at", false)
                   .limit(100)
                   .get();
    }
    
    // 获取最近的日志
    ResultSet recentLogs(int limit = 100) {
        return query().orderBy("created_at", false).limit(limit).get();
    }
};

// 打印机模型
class PrinterModel : public Model {
public:
    PrinterModel(Database* db) : Model(db, "printers") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_TEXT("name"),
            DB_FIELD_TEXT("ip_address"),
            DB_FIELD_TEXT("mac_address"),
            DB_FIELD_TEXT("location"),
            DB_FIELD_TEXT("status"),
            DB_FIELD_INTEGER("ink_level"),
            DB_FIELD_INTEGER("paper_level")
        };
    }
    
    // 创建打印机
    bool create(const std::string& name, const std::string& ip_address, 
                const std::string& mac_address, const std::string& status = "online") {
        RowData data;
        data["name"] = name;
        data["ip_address"] = ip_address;
        data["mac_address"] = mac_address;
        data["location"] = "";
        data["status"] = status;
        data["ink_level"] = "100";
        data["paper_level"] = "100";
        return insert(data);
    }
    
    // 更新打印机配置
    bool updateConfig(const std::string& id, const std::string& name, const std::string& ip_address,
                const std::string& mac_address, const std::string& location) {
        RowData data;
        data["name"] = name;
        data["ip_address"] = ip_address;
        data["mac_address"] = mac_address;
        data["location"] = location;
        return update(data, "id = " + id);
    }
    
    // 根据IP地址查找
    RowData findByIPAddress(const std::string& ip_address) {
        return query().where("ip_address = '" + db_->escapeString(ip_address) + "'").first();
    }
};

// 打印任务模型
class PrintJobModel : public Model {
public:
    PrintJobModel(Database* db) : Model(db, "print_jobs") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_TEXT("document_name"),
            DB_FIELD_TEXT("status"),
            DB_FIELD_INTEGER("copies"),
            DB_FIELD_TEXT("created_at"),
            DB_FIELD_TEXT("completed_at")
        };
    }
    
    // 创建打印任务
    bool create(const std::string& document_name, const std::string& status = "pending", int copies = 1) {
        RowData data;
        data["document_name"] = document_name;
        data["status"] = status;
        data["copies"] = std::to_string(copies);
        data["created_at"] = getCurrentTimestamp();
        data["completed_at"] = "";
        return insert(data);
    }
    
    // 更新任务状态
    bool updateStatus(const std::string& id, const std::string& status) {
        RowData data;
        data["status"] = status;
        if (status == "completed") {
            data["completed_at"] = "";
        }
        return update(data, "id = " + id);
    }
};

// 地址簿模型
class AddressBookModel : public Model {
public:
    AddressBookModel(Database* db) : Model(db, "address_book") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_TEXT("name"),
            DB_FIELD_TEXT("email"),
            DB_FIELD_TEXT("phone"),
            DB_FIELD_TEXT("fax"),
            DB_FIELD_TEXT("address"),
            DB_FIELD_TEXT("company"),
            DB_FIELD_TEXT("department"),
            DB_FIELD_TEXT("notes"),
            DB_FIELD_TEXT("created_by"),
            DB_FIELD_TEXT("created_at"),
            DB_FIELD_TEXT("updated_at")
        };
    }
    
    // 创建联系人
    bool create(const std::string& name, const std::string& email, const std::string& phone,
                const std::string& address = "", const std::string& company = "",
                const std::string& department = "", const std::string& notes = "",
                const std::string& created_by = "system") {
        RowData data;
        data["name"] = name;
        data["email"] = email;
        data["phone"] = phone;
        data["fax"] = "";
        data["address"] = address;
        data["company"] = company;
        data["department"] = department;
        data["notes"] = notes;
        data["created_by"] = created_by;
        data["created_at"] = getCurrentTimestamp();
        data["updated_at"] = getCurrentTimestamp();
        return insert(data);
    }
    
    // 按姓名搜索
    ResultSet searchByName(const std::string& name) {
        return query().where("name LIKE '%" + db_->escapeString(name) + "%'").get();
    }
    
    // 按公司搜索
    ResultSet searchByCompany(const std::string& company) {
        return query().where("company LIKE '%" + db_->escapeString(company) + "%'").get();
    }
    
    // 按部门搜索
    ResultSet searchByDepartment(const std::string& department) {
        return query().where("department = '" + db_->escapeString(department) + "'").get();
    }
    
    // 更新联系人
    bool updateContact(const std::string& id, const std::string& name, const std::string& email,
                       const std::string& phone, const std::string& address = "",
                       const std::string& company = "", const std::string& department = "",
                       const std::string& notes = "") {
        RowData data;
        data["name"] = name;
        data["email"] = email;
        data["phone"] = phone;
        data["address"] = address;
        data["company"] = company;
        data["department"] = department;
        data["notes"] = notes;
        data["updated_at"] = getCurrentTimestamp();
        return updateById(safeSto<int64_t>(id, 0), data);
    }
    
    // 删除联系人
    bool removeContact(const std::string& id) {
        return remove("id = " + id);
    }
};

// 安全审计日志模型（增强版）
class SecurityAuditLogModel : public Model {
public:
    SecurityAuditLogModel(Database* db) : Model(db, "security_audit_logs") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_INTEGER("user_id"),
            DB_FIELD_TEXT("username"),
            DB_FIELD_TEXT("action"),
            DB_FIELD_TEXT("resource_type"),
            DB_FIELD_INTEGER("resource_id"),
            DB_FIELD_TEXT("ip_address"),
            DB_FIELD_TEXT("user_agent"),
            DB_FIELD_TEXT("result"),
            DB_FIELD_TEXT("failure_reason"),
            DB_FIELD_TEXT("timestamp")
        };
    }
    
    // 记录安全事件
    bool log(int64_t user_id, const std::string& username, const std::string& action,
            const std::string& resource_type = "", int64_t resource_id = 0,
            const std::string& ip_address = "", const std::string& user_agent = "",
            const std::string& result = "success", const std::string& failure_reason = "") {
        RowData data;
        data["user_id"] = std::to_string(user_id);
        data["username"] = username;
        data["action"] = action;
        data["resource_type"] = resource_type;
        data["resource_id"] = std::to_string(resource_id);
        data["ip_address"] = ip_address;
        data["user_agent"] = user_agent;
        data["result"] = result;
        data["failure_reason"] = failure_reason;
        data["timestamp"] = "";
        return insert(data);
    }
    
    // 查询用户日志
    ResultSet findByUser(int64_t user_id, int limit = 100) {
        return query().where("user_id = " + std::to_string(user_id))
                   .orderBy("timestamp", false)
                   .limit(limit)
                   .get();
    }
    
    // 查询失败的登录尝试
    ResultSet findFailedLogins(const std::string& username, int limit = 10) {
        return query().where("username = '" + username + "' AND result = 'failure'")
                   .orderBy("timestamp", false)
                   .limit(limit)
                   .get();
    }
    
    // 查询最近的日志
    ResultSet recentLogs(int limit = 100) {
        return query().orderBy("timestamp", false).limit(limit).get();
    }
    
    // 按操作类型查询
    ResultSet findByAction(const std::string& action, int limit = 100) {
        return query().where("action = '" + db_->escapeString(action) + "'")
                   .orderBy("timestamp", false)
                   .limit(limit)
                   .get();
    }
};

// IP访问控制模型
class IpAccessControlModel : public Model {
public:
    IpAccessControlModel(Database* db) : Model(db, "ip_access_control") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_TEXT("ip_address"),
            DB_FIELD_TEXT("ip_range"),
            DB_FIELD_TEXT("access_type"),
            DB_FIELD_TEXT("description"),
            DB_FIELD_TEXT("created_by"),
            DB_FIELD_TEXT("created_at")
        };
    }
    
    // 添加IP规则
    bool addRule(const std::string& ip_address, const std::string& access_type,
                 const std::string& description = "", const std::string& created_by = "system") {
        RowData data;
        data["ip_address"] = ip_address;
        data["ip_range"] = "";
        data["access_type"] = access_type;  // allow, deny, block
        data["description"] = description;
        data["created_by"] = created_by;
        data["created_at"] = getCurrentTimestamp();
        return insert(data);
    }
    
    // 添加IP范围规则
    bool addRangeRule(const std::string& ip_range, const std::string& access_type,
                      const std::string& description = "", const std::string& created_by = "system") {
        RowData data;
        data["ip_address"] = "";
        data["ip_range"] = ip_range;
        data["access_type"] = access_type;
        data["description"] = description;
        data["created_by"] = created_by;
        data["created_at"] = getCurrentTimestamp();
        return insert(data);
    }
    
    // 检查IP访问权限
    bool checkAccess(const std::string& ip_address) {
        // 先检查是否被阻止
        ResultSet blocked = query().where("(ip_address = '" + ip_address + "' OR ip_range != '') AND access_type = 'block'").get();
        if (!blocked.getRows().empty()) return false;
        
        // 检查是否被允许
        ResultSet allowed = query().where("(ip_address = '" + ip_address + "' OR ip_range != '') AND access_type = 'allow'").get();
        if (!allowed.getRows().empty()) return true;
        
        // 默认拒绝
        return false;
    }
    
    // 删除规则
    bool removeRule(const std::string& id) {
        return remove("id = " + id);
    }
};

// 证书管理模型
class CertificateModel : public Model {
public:
    CertificateModel(Database* db) : Model(db, "certificates") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_TEXT("certificate_name"),
            DB_FIELD_TEXT("certificate_type"),
            DB_FIELD_TEXT("issuer"),
            DB_FIELD_TEXT("subject"),
            DB_FIELD_TEXT("serial_number"),
            DB_FIELD_TEXT("valid_from"),
            DB_FIELD_TEXT("valid_until"),
            DB_FIELD_TEXT("status"),
            DB_FIELD_TEXT("certificate_pem"),
            DB_FIELD_TEXT("private_key_pem"),
            DB_FIELD_TEXT("purpose"),
            DB_FIELD_TEXT("created_by"),
            DB_FIELD_TEXT("created_at"),
            DB_FIELD_TEXT("updated_at")
        };
    }
    
    // 创建证书
    bool create(const std::string& name, const std::string& type, const std::string& issuer,
                const std::string& subject, const std::string& serial, 
                const std::string& valid_from, const std::string& valid_until,
                const std::string& cert_pem, const std::string& key_pem = "",
                const std::string& purpose = "server", const std::string& created_by = "system") {
        RowData data;
        data["certificate_name"] = name;
        data["certificate_type"] = type;  // self_signed, ca, server, client
        data["issuer"] = issuer;
        data["subject"] = subject;
        data["serial_number"] = serial;
        data["valid_from"] = valid_from;
        data["valid_until"] = valid_until;
        data["status"] = "active";
        data["certificate_pem"] = cert_pem;
        data["private_key_pem"] = key_pem;
        data["purpose"] = purpose;
        data["created_by"] = created_by;
        data["created_at"] = getCurrentTimestamp();
        data["updated_at"] = getCurrentTimestamp();
        return insert(data);
    }
    
    // 按类型查询
    ResultSet findByType(const std::string& type) {
        return query().where("certificate_type = '" + db_->escapeString(type) + "'").get();
    }
    
    // 按状态查询
    ResultSet findByStatus(const std::string& status) {
        return query().where("status = '" + db_->escapeString(status) + "'").get();
    }
    
    // 检查证书有效期
    ResultSet findExpiringCertificates(int days = 30) {
        (void)days; // 避免未使用参数警告
        return query().where("status = 'active' AND date(valid_until) <= date('now', '+30 days')").get();
    }
    
    // 更新证书状态
    bool updateStatus(const std::string& id, const std::string& status) {
        RowData data;
        data["status"] = status;
        data["updated_at"] = getCurrentTimestamp();
        return updateById(safeSto<int64_t>(id, 0), data);
    }
    
    // 更新证书
    bool updateCertificate(const std::string& id, const std::string& cert_pem) {
        RowData data;
        data["certificate_pem"] = cert_pem;
        data["updated_at"] = getCurrentTimestamp();
        return updateById(safeSto<int64_t>(id, 0), data);
    }
    
    // 删除证书
    bool removeCertificate(const std::string& id) {
        return remove("id = " + id);
    }
};

// 网络配置模型
class NetworkConfigModel : public Model {
public:
    NetworkConfigModel(Database* db) : Model(db, "network_config") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_TEXT("interface_name"),
            DB_FIELD_TEXT("ip_address"),
            DB_FIELD_TEXT("netmask"),
            DB_FIELD_TEXT("gateway"),
            DB_FIELD_TEXT("dns_primary"),
            DB_FIELD_TEXT("dns_secondary"),
            DB_FIELD_TEXT("dhcp_enabled"),
            DB_FIELD_TEXT("mtu"),
            DB_FIELD_TEXT("status")
        };
    }
    
    // 创建网络配置
    bool create(const std::string& interface_name, const std::string& ip_address,
                const std::string& netmask, const std::string& gateway,
                const std::string& dns_primary, const std::string& dns_secondary = "",
                bool dhcp_enabled = true, int mtu = 1500) {
        RowData data;
        data["interface_name"] = interface_name;
        data["ip_address"] = ip_address;
        data["netmask"] = netmask;
        data["gateway"] = gateway;
        data["dns_primary"] = dns_primary;
        data["dns_secondary"] = dns_secondary;
        data["dhcp_enabled"] = dhcp_enabled ? "1" : "0";
        data["mtu"] = std::to_string(mtu);
        data["status"] = "active";
        return insert(data);
    }
    
    // 根据接口名称查找
    RowData findByInterface(const std::string& interface_name) {
        return query().where("interface_name = '" + db_->escapeString(interface_name) + "'").first();
    }
    
    // 更新网络配置
    bool updateConfig(const std::string& id, const std::string& ip_address,
                     const std::string& netmask, const std::string& gateway,
                     const std::string& dns_primary, const std::string& dns_secondary = "",
                     bool dhcp_enabled = true, int mtu = 1500) {
        RowData data;
        data["ip_address"] = ip_address;
        data["netmask"] = netmask;
        data["gateway"] = gateway;
        data["dns_primary"] = dns_primary;
        data["dns_secondary"] = dns_secondary;
        data["dhcp_enabled"] = dhcp_enabled ? "1" : "0";
        data["mtu"] = std::to_string(mtu);
        return updateById(std::stoll(id), data);
    }
    
    // 启用/禁用接口
    bool setStatus(const std::string& id, const std::string& status) {
        RowData data;
        data["status"] = status;
        return updateById(std::stoll(id), data);
    }
    
    // 测试网络连通性
    bool testConnection(const std::string& gateway) {
        // 简化实现，实际应该调用系统 ping 命令
        return !gateway.empty();
    }
};

// 蓝牙配置模型
class BluetoothConfigModel : public Model {
public:
    BluetoothConfigModel(Database* db) : Model(db, "bluetooth_config") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_TEXT("device_name"),
            DB_FIELD_TEXT("mac_address"),
            DB_FIELD_TEXT("pin_code"),
            DB_FIELD_TEXT("discoverable"),
            DB_FIELD_TEXT("pairable"),
            DB_FIELD_INTEGER("discoverable_timeout"),
            DB_FIELD_TEXT("status")
        };
    }
    
    bool create(const std::string& device_name, const std::string& mac_address = "",
                const std::string& pin_code = "0000", bool discoverable = true,
                bool pairable = true, int discoverable_timeout = 180) {
        RowData data;
        data["device_name"] = device_name;
        data["mac_address"] = mac_address;
        data["pin_code"] = pin_code;
        data["discoverable"] = discoverable ? "1" : "0";
        data["pairable"] = pairable ? "1" : "0";
        data["discoverable_timeout"] = std::to_string(discoverable_timeout);
        data["status"] = "active";
        return insert(data);
    }
    
    bool updateConfig(const std::string& id, const std::string& device_name,
                     const std::string& pin_code, bool discoverable, bool pairable,
                     int discoverable_timeout) {
        RowData data;
        data["device_name"] = device_name;
        data["pin_code"] = pin_code;
        data["discoverable"] = discoverable ? "1" : "0";
        data["pairable"] = pairable ? "1" : "0";
        data["discoverable_timeout"] = std::to_string(discoverable_timeout);
        return updateById(std::stoll(id), data);
    }
};

// Bonjour/mDNS配置模型
class BonjourConfigModel : public Model {
public:
    BonjourConfigModel(Database* db) : Model(db, "bonjour_config") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_TEXT("service_name"),
            DB_FIELD_TEXT("service_type"),
            DB_FIELD_TEXT("host_name"),
            DB_FIELD_INTEGER("port"),
            DB_FIELD_TEXT("domain"),
            DB_FIELD_TEXT("txt_records"),
            DB_FIELD_TEXT("status")
        };
    }
    
    bool create(const std::string& service_name, const std::string& service_type,
                const std::string& host_name, int port, const std::string& domain = "local",
                const std::string& txt_records = "") {
        RowData data;
        data["service_name"] = service_name;
        data["service_type"] = service_type;
        data["host_name"] = host_name;
        data["port"] = std::to_string(port);
        data["domain"] = domain;
        data["txt_records"] = txt_records;
        data["status"] = "active";
        return insert(data);
    }
    
    bool updateConfig(const std::string& id, const std::string& service_name,
                     const std::string& service_type, const std::string& host_name,
                     int port, const std::string& txt_records) {
        RowData data;
        data["service_name"] = service_name;
        data["service_type"] = service_type;
        data["host_name"] = host_name;
        data["port"] = std::to_string(port);
        data["txt_records"] = txt_records;
        return updateById(std::stoll(id), data);
    }
};

// SMTP配置模型
class SmtpConfigModel : public Model {
public:
    SmtpConfigModel(Database* db) : Model(db, "smtp_config") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_TEXT("server_name"),
            DB_FIELD_INTEGER("port"),
            DB_FIELD_TEXT("username"),
            DB_FIELD_TEXT("password"),
            DB_FIELD_TEXT("sender_email"),
            DB_FIELD_TEXT("encryption"),
            DB_FIELD_TEXT("authentication"),
            DB_FIELD_TEXT("status")
        };
    }
    
    bool create(const std::string& server_name, int port, const std::string& username,
                const std::string& password, const std::string& sender_email,
                const std::string& encryption = "ssl", bool authentication = true) {
        RowData data;
        data["server_name"] = server_name;
        data["port"] = std::to_string(port);
        data["username"] = username;
        data["password"] = password;
        data["sender_email"] = sender_email;
        data["encryption"] = encryption;
        data["authentication"] = authentication ? "1" : "0";
        data["status"] = "active";
        return insert(data);
    }
    
    bool updateConfig(const std::string& id, const std::string& server_name, int port,
                     const std::string& username, const std::string& password,
                     const std::string& sender_email, const std::string& encryption,
                     bool authentication) {
        RowData data;
        data["server_name"] = server_name;
        data["port"] = std::to_string(port);
        data["username"] = username;
        data["password"] = password;
        data["sender_email"] = sender_email;
        data["encryption"] = encryption;
        data["authentication"] = authentication ? "1" : "0";
        return updateById(std::stoll(id), data);
    }
};

// FTP配置模型
class FtpConfigModel : public Model {
public:
    FtpConfigModel(Database* db) : Model(db, "ftp_config") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_TEXT("server_name"),
            DB_FIELD_INTEGER("port"),
            DB_FIELD_TEXT("username"),
            DB_FIELD_TEXT("password"),
            DB_FIELD_TEXT("root_directory"),
            DB_FIELD_TEXT("mode"),
            DB_FIELD_TEXT("anonymous_enabled"),
            DB_FIELD_TEXT("status")
        };
    }
    
    bool create(const std::string& server_name, int port, const std::string& username,
                const std::string& password, const std::string& root_directory = "/ftp",
                const std::string& mode = "passive", bool anonymous_enabled = false) {
        RowData data;
        data["server_name"] = server_name;
        data["port"] = std::to_string(port);
        data["username"] = username;
        data["password"] = password;
        data["root_directory"] = root_directory;
        data["mode"] = mode;
        data["anonymous_enabled"] = anonymous_enabled ? "1" : "0";
        data["status"] = "active";
        return insert(data);
    }
    
    bool updateConfig(const std::string& id, const std::string& server_name, int port,
                     const std::string& username, const std::string& password,
                     const std::string& root_directory, const std::string& mode,
                     bool anonymous_enabled) {
        RowData data;
        data["server_name"] = server_name;
        data["port"] = std::to_string(port);
        data["username"] = username;
        data["password"] = password;
        data["root_directory"] = root_directory;
        data["mode"] = mode;
        data["anonymous_enabled"] = anonymous_enabled ? "1" : "0";
        return updateById(std::stoll(id), data);
    }
};

// SNTP/NTP配置模型
class SntpConfigModel : public Model {
public:
    SntpConfigModel(Database* db) : Model(db, "sntp_config") {}
    
    std::vector<Field> fields() override {
        return {
            DB_FIELD_TEXT("server_address"),
            DB_FIELD_INTEGER("port"),
            DB_FIELD_INTEGER("poll_interval"),
            DB_FIELD_INTEGER("timeout"),
            DB_FIELD_TEXT("timezone"),
            DB_FIELD_TEXT("status")
        };
    }
    
    bool create(const std::string& server_address, int port = 123, int poll_interval = 3600,
                int timeout = 5, const std::string& timezone = "UTC") {
        RowData data;
        data["server_address"] = server_address;
        data["port"] = std::to_string(port);
        data["poll_interval"] = std::to_string(poll_interval);
        data["timeout"] = std::to_string(timeout);
        data["timezone"] = timezone;
        data["status"] = "active";
        return insert(data);
    }
    
    bool updateConfig(const std::string& id, const std::string& server_address, int port,
                     int poll_interval, int timeout, const std::string& timezone) {
        RowData data;
        data["server_address"] = server_address;
        data["port"] = std::to_string(port);
        data["poll_interval"] = std::to_string(poll_interval);
        data["timeout"] = std::to_string(timeout);
        data["timezone"] = timezone;
        return updateById(std::stoll(id), data);
    }
};

// LDAP配置模型
class LdapConfigModel : public Model {
public:
    LdapConfigModel(Database* db) : Model(db, "ldap_config") {}

    std::vector<Field> fields() override {
        return {
            DB_FIELD_TEXT("server_url"),
            DB_FIELD_INTEGER("port"),
            DB_FIELD_TEXT("base_dn"),
            DB_FIELD_TEXT("bind_dn"),
            DB_FIELD_TEXT("bind_password"),
            DB_FIELD_TEXT("search_filter"),
            DB_FIELD_TEXT("user_id_attribute"),
            DB_FIELD_TEXT("email_attribute"),
            DB_FIELD_TEXT("name_attribute"),
            DB_FIELD_INTEGER("timeout"),
            DB_FIELD_TEXT("use_ssl"),
            DB_FIELD_TEXT("use_tls"),
            DB_FIELD_TEXT("enabled"),
            DB_FIELD_TEXT("status")
        };
    }

    bool create(const std::string& server_url, int port = 389,
                const std::string& base_dn = "dc=example,dc=com",
                const std::string& bind_dn = "cn=admin,dc=example,dc=com",
                const std::string& bind_password = "",
                const std::string& search_filter = "(objectClass=person)",
                const std::string& user_id_attribute = "uid",
                const std::string& email_attribute = "mail",
                const std::string& name_attribute = "cn",
                int timeout = 10,
                bool use_ssl = false,
                bool use_tls = false,
                bool enabled = false) {
        RowData data;
        data["server_url"] = server_url;
        data["port"] = std::to_string(port);
        data["base_dn"] = base_dn;
        data["bind_dn"] = bind_dn;
        data["bind_password"] = bind_password;
        data["search_filter"] = search_filter;
        data["user_id_attribute"] = user_id_attribute;
        data["email_attribute"] = email_attribute;
        data["name_attribute"] = name_attribute;
        data["timeout"] = std::to_string(timeout);
        data["use_ssl"] = use_ssl ? "1" : "0";
        data["use_tls"] = use_tls ? "1" : "0";
        data["enabled"] = enabled ? "1" : "0";
        data["status"] = "active";
        return insert(data);
    }

    bool updateConfig(const std::string& id, const std::string& server_url, int port,
                     const std::string& base_dn, const std::string& bind_dn,
                     const std::string& bind_password, const std::string& search_filter,
                     const std::string& user_id_attribute, const std::string& email_attribute,
                     const std::string& name_attribute, int timeout,
                     bool use_ssl, bool use_tls, bool enabled) {
        RowData data;
        data["server_url"] = server_url;
        data["port"] = std::to_string(port);
        data["base_dn"] = base_dn;
        data["bind_dn"] = bind_dn;
        data["bind_password"] = bind_password;
        data["search_filter"] = search_filter;
        data["user_id_attribute"] = user_id_attribute;
        data["email_attribute"] = email_attribute;
        data["name_attribute"] = name_attribute;
        data["timeout"] = std::to_string(timeout);
        data["use_ssl"] = use_ssl ? "1" : "0";
        data["use_tls"] = use_tls ? "1" : "0";
        data["enabled"] = enabled ? "1" : "0";

        try {
            return updateById(std::stoll(id), data);
        } catch (const std::exception& e) {
            return false;
        }
    }

    // 测试LDAP连接
    bool testConnection() {
        ResultSet configs = all();
        if (configs.getRows().empty()) {
            return false;
        }
        RowData config = configs.getRows()[0];
        if (config["enabled"] != "1") {
            return false;
        }
        // 简化实现，实际应该使用LDAP库连接测试
        return !config["server_url"].empty();
    }
};

#endif // RESTFUL_MODEL_H
