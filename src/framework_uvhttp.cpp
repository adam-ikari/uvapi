// 创建 JSON 字段
cJSON* createJsonField(void* instance, size_t offset, FieldType type, const FieldDefinition* field_def = nullptr) {
    if (!instance) return nullptr;
    
    char* field_ptr = static_cast<char*>(instance) + offset;
    
    switch (type) {
        case FieldType::STRING:
            return cJSON_CreateString(reinterpret_cast<std::string*>(field_ptr)->c_str());
        case FieldType::INT8:
            return cJSON_CreateNumber(*reinterpret_cast<int8_t*>(field_ptr));
        case FieldType::INT16:
            return cJSON_CreateNumber(*reinterpret_cast<int16_t*>(field_ptr));
        case FieldType::INT32:
            return cJSON_CreateNumber(*reinterpret_cast<int32_t*>(field_ptr));
        case FieldType::INT64:
            return cJSON_CreateNumber(static_cast<double>(*reinterpret_cast<int64_t*>(field_ptr)));
        case FieldType::UINT8:
            return cJSON_CreateNumber(*reinterpret_cast<uint8_t*>(field_ptr));
        case FieldType::UINT16:
            return cJSON_CreateNumber(*reinterpret_cast<uint16_t*>(field_ptr));
        case FieldType::UINT32:
            return cJSON_CreateNumber(*reinterpret_cast<uint32_t*>(field_ptr));
        case FieldType::UINT64:
            return cJSON_CreateNumber(static_cast<double>(*reinterpret_cast<uint64_t*>(field_ptr)));
        case FieldType::FP32:
            return cJSON_CreateNumber(*reinterpret_cast<float*>(field_ptr));
        case FieldType::FP64:
            return cJSON_CreateNumber(*reinterpret_cast<double*>(field_ptr));
        case FieldType::BOOL:
            return cJSON_CreateBool(*reinterpret_cast<bool*>(field_ptr));
        case FieldType::DATE:
        case FieldType::DATETIME:
        case FieldType::EMAIL:
        case FieldType::URL:
        case FieldType::UUID:
            return cJSON_CreateString(reinterpret_cast<std::string*>(field_ptr)->c_str());
        case FieldType::OBJECT: {
            if (field_def && field_def->nested_schema) {
                // 嵌套对象：递归调用嵌套 schema 的 toJson 方法
                std::string nested_json = field_def->nested_schema->toJson(field_ptr);
                return cJSON_Parse(nested_json.c_str());
            }
            return cJSON_CreateNull();
        }
        case FieldType::ARRAY: {
            if (field_def && field_def->item_type != FieldType::STRING) {
                // 数组：使用 serializeArray 辅助方法
                const std::vector<std::string>* vec = reinterpret_cast<std::vector<std::string>*>(field_ptr);
                if (vec) {
                    return serializeArray<std::string>(*vec, field_def);
                }
            }
            return cJSON_CreateNull();
        }
        case FieldType::CUSTOM: {
            if (field_def && field_def->custom_handler) {
                // 自定义类型：使用自定义处理器序列化
                std::string serialized = field_def->custom_handler->serialize(instance, offset);
                return cJSON_Parse(serialized.c_str());
            }
            return cJSON_CreateNull();
        }
    }
    
    return nullptr;
}