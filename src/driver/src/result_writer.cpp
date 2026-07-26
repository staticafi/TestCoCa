#include <driver/result_writer.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree_fwd.hpp>
#include <cmath>
#include <string>

void write_result_json(const std::string& filename, const test_result& result)
{
    boost::property_tree::ptree root;

    float coverage = std::round(result.score * 10000.0f) / 10000.0f;

    root.put("coverage", coverage);

    boost::property_tree::ptree coverage_node;
    for (const auto& [key, value] : result.branch_coverage) {
        std::string key_str = std::to_string(key);
        switch (value) {
            case instrumentation::FALSE:
                coverage_node.put<std::string>(key_str, "FALSE");
                break;
            case instrumentation::TRUE:
                coverage_node.put<std::string>(key_str, "TRUE");
                break;
            case instrumentation::BOTH:
                coverage_node.put<std::string>(key_str, "BOTH");
                break;
            default:;
        }
    }

    root.add_child("coverage_map", coverage_node);

    write_json(filename, root, std::locale(), true);
}
