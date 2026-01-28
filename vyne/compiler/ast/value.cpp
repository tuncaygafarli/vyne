#include "value.h"

Value& Value::setReadOnly(){
    isReadOnly = true;
    return *this;
}

void Value::print(std::ostream& os) const {
    switch(data.index()){
        case 0 :
            os << "null";
            break;
        case 1 :
            os << std::get<double>(data);
            break;
        case 2 :
            os << "\"" << *std::get<std::shared_ptr<std::string>>(data) << "\"";
            break;
        case 3 : {
            const auto& list = *std::get<std::shared_ptr<std::vector<Value>>>(data);

            os << "[";
            for (size_t i = 0; i < list.size(); ++i) {
                list[i].print(os);
                if (i < list.size() - 1) os << ", ";
            }
            os << "]";
            break;
        }

        case 5:
            os << "<function>";
            break;
        case 6:
            os << "<module '" << std::get<std::string>(data) << "'>";
            break;
        default:
            os << "<unknown>";
            break; 
    }
}

size_t Value::getBytes() const {
    switch(data.index()){
        case 1:
            return sizeof(double);
        case 2: 
            return std::get<std::shared_ptr<std::string>>(data)->length() * sizeof(char);
        case 3: {
            size_t total = 0;

            const auto& list = *std::get<std::shared_ptr<std::vector<Value>>>(data);
            for (const auto& v : list) {
                total += v.getBytes();
            }

            return total;
        }

        default :
            return 0;
    }
}

bool Value::equals(const Value& other) const {
    if (getType() != other.getType()) return false;

    switch(getType()) {
        case 0: return true;
        case 1: return asNumber() == other.asNumber();
        case 2: return asString() == other.asString();
        default: return false;
    }
}

std::string Value::toString() const {
    switch(data.index()) {
        case 1: {
            std::string s = std::to_string(std::get<double>(data));
            s.erase(s.find_last_not_of('0') + 1, std::string::npos);
            if(s.back() == '.') s.pop_back();
            return s;
        }
        case 2:
            return *std::get<std::shared_ptr<std::string>>(data);
        case 0: 
            return "null";
        default: {
            std::stringstream ss;
            this->print(ss);
            return ss.str();
        }
    }
}

int Value::toNumber() const {
    switch(data.index()){
        case 0 : return 0;
        case 1 :  return std::get<double>(data);
        case 2 : {
            try {
            return std::stod(*std::get<std::shared_ptr<std::string>>(data));
        } catch (...) {
            return 0.0; 
        }
        }
        default :
            return 0;
    }
};

bool Value::isTruthy() const {
    switch (data.index()) {
        case 0: return false;
        case 1: return std::get<double>(data) != 0;
        case 2: return !std::get<std::shared_ptr<std::string>>(data)->empty();
        default: return true; 
    }
}