/*
 * This was written in a short amount of time. Please take this into account
 * when reading through this code.
 *
 * Some general aspects: I chose cpprest (casablance) since i fits into C++11
 * code nicely. When choosing cpprest I was not aware of their not so nice
 * exception handling. This should be improved.
 *
 * This code is neither written for performance nor modularity. It's wirtten
 * solely to do the job, nothing more. Not even to do the job good.
 *
 */
#include <iostream>
#include <fstream>
#include <string>
#include <ios>
#include <cpprest/json.h>
#include <cpprest/http_client.h>
#include <cpprest/details/http_helpers.h>

bool is_json_response(const web::http::http_response& response) {
    utility::string_t content, charset = web::http::details::charset_types::utf8;
    web::http::details::parse_content_type_and_charset(
        response.headers().content_type(), content, charset);
    return !content.empty() && web::http::details::is_content_type_json(content);
}

pplx::task<web::json::value> get_json(const web::uri& url) {
    web::http::client::http_client client(url);
    web::http::http_request request(web::http::methods::GET);
    std::cout << "asking for data from: " << url.to_string() << std::endl;
    return client.request(request).then([](web::http::http_response response) {
            std::cout << "data received" << std::endl;
            if ((response.status_code() == web::http::status_codes::OK) &&
                is_json_response(response)) {
                /**
                 * crashes on malformed json without throwing an exception
                 */
                return response.extract_json();
            }
            std::cout << "received invalid data" << std::endl;
            return pplx::create_task([] { return web::json::value(); });
        });
}

utility::string_t escape_csv(const utility::string_t& str) {
    std::ostringstream os;
    for (const char& c: str) {
        if (c == '"') {
            os << "\"\"";
        } else {
            os << c;
        }
    }
    return os.str();
}

utility::string_t json_value_to_csv_entry(const web::json::value& jsonValue) {
    /*
     * currently only numbers and strings are required. everything else -> ""
     */
    switch (jsonValue.type()) {
        case web::json::value::Number:
          return "\"" + jsonValue.serialize() + "\"";
        case web::json::value::String:
          return "\"" + escape_csv(jsonValue.as_string()) + "\"";
        default:
          return "";
    }
}

void json_to_goeuro_csv_line(web::json::value& jsonValue,
                             std::ofstream& os) {
    web::json::value geo_position;
    if (!jsonValue.is_object()) {
        return;
    }
    web::json::object& jsonObject = jsonValue.as_object();
    os << json_value_to_csv_entry(jsonObject["_id"]) << ",";
    os << json_value_to_csv_entry(jsonObject["name"]) << ",";
    os << json_value_to_csv_entry(jsonObject["type"]) << ",";
    geo_position = jsonObject["geo_position"];
    if (geo_position.is_object()) {
        os << json_value_to_csv_entry(geo_position.as_object()["latitude"]) << ",";
        os << json_value_to_csv_entry(geo_position.as_object()["longitude"]);
    } else {
        os << json_value_to_csv_entry(web::json::value::value()) << ",";
        os << json_value_to_csv_entry(web::json::value::value());
    }
    os << "\n";
}

void write_json_to_file(const web::json::value& jsonValue,
                        const std::string& filename) {
    if (jsonValue.is_array()) {
        std::ofstream os(filename, std::ofstream::out);
        if (os.fail()) {
            std::cout << "could not write to file: " << filename << std::endl;
            return;
        }
        std::cout << "wirting data to: " << filename << std::endl;
        for (auto elem: jsonValue.as_array()) {
            json_to_goeuro_csv_line(elem, os);
        }
    }
}

int main(int argc, const char* argv[]) {
    if (argc != 3) {
        std::cout << "usage: " << argv[0] << " SEARCH_STRING OUTPUT_FILE" << std::endl;
        return 1;
    }
    try {
        web::uri_builder builder(U("http://api.goeuro.com/api/v2/position/suggest/en/"));
        builder.append_path(U(argv[1]));
        get_json(web::uri(builder.to_uri())).then(
            [=](web::json::value jsonValue) {
                write_json_to_file(jsonValue, std::string(argv[2]));
            }).wait();
    } catch (web::http::http_exception& e) {
        std::cout << "could not reach host" << std::endl;
    }
    return 1;
}
