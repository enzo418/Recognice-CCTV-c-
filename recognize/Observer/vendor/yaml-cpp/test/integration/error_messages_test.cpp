#include "gtest/gtest.h"
#include "yaml-cpp/exceptions.h"
#include "yaml-cpp/yaml.h"  // IWYU pragma: keep

#define EXPECT_THROW_EXCEPTION(exception_type, statement, message) \
    ASSERT_THROW(statement, exception_type);                       \
    try {                                                          \
        statement;                                                 \
    } catch (const exception_type& e) {                            \
        EXPECT_EQ(e.msg, message);                                 \
    }

namespace YAML {
    namespace {

        TEST(ErrorMessageTest, BadSubscriptErrorMessage) {
            const char* example_yaml =
                "first:\n"
                "   second: 1\n"
                "   third: 2\n";

            Node doc = Load(example_yaml);

            // Test that printable key is part of error message
            EXPECT_THROW_EXCEPTION(
                YAML::BadSubscript, doc["first"]["second"]["fourth"],
                "operator[] call on a scalar (key: \"fourth\")");

            EXPECT_THROW_EXCEPTION(YAML::BadSubscript,
                                   doc["first"]["second"][37],
                                   "operator[] call on a scalar (key: \"37\")");

            // Non-printable key is not included in error message
            EXPECT_THROW_EXCEPTION(YAML::BadSubscript,
                                   doc["first"]["second"][std::vector<int>()],
                                   "operator[] call on a scalar");

            EXPECT_THROW_EXCEPTION(YAML::BadSubscript,
                                   doc["first"]["second"][Node()],
                                   "operator[] call on a scalar");
        }

        TEST(ErrorMessageTest, Ex9_1_TypedBadConversionErrorMessage) {
            const char* example_yaml =
                "first:\n"
                "   second: 1\n"
                "   third: 2\n";

            const Node doc = Load(example_yaml);

            // Non-printable key is not included in error message
            EXPECT_THROW_EXCEPTION(YAML::TypedBadConversion<int>,
                                   doc["first"].as<int>(), "bad conversion");

            // mark should be right
            try {
                doc["first"].as<int>();
            } catch (YAML::TypedBadConversion<int> ex) {
                ASSERT_EQ(ex.mark.pos, 10);  // 10 = parsed is before "second"
                ASSERT_EQ(ex.mark.line, 1);
                ASSERT_EQ(ex.mark.column, 3);  // 3rd space in 2nd line
            }
        }

        TEST(ErrorMessageTest, Ex9_2_KeyNotFoundErrorMessage) {
            const char* example_yaml =
                "first:\n"
                "   second: 1\n"
                "   third: 2\n"
                "main:\n"
                "   main2:\n"
                "      one: 1";

            const Node doc = Load(example_yaml);

            // Test that printable key is part of error message
            EXPECT_THROW_EXCEPTION(YAML::KeyNotFound, doc["first"]["fourth"],
                                   "key not found: \"fourth\"");

            EXPECT_THROW_EXCEPTION(YAML::KeyNotFound,
                                   doc["first"][37].as<int>(),
                                   "key not found: \"37\"");

            EXPECT_THROW_EXCEPTION(
                YAML::KeyNotFound, doc["first"][std::vector<int>()].as<int>(),
                "invalid node; this may result from using a map "
                "iterator as a sequence iterator, or vice-versa");
        }

        TEST(ErrorMessageTest, Ex9_3_KeyNotFoundMarks) {
            const char* example_yaml =
                "first:\n"
                "   second: 1\n"
                "   third: 2\n"
                "main:\n"
                "   main2:\n"
                "      one: 1";

            const Node doc = Load(example_yaml);

            // check marks on "first"
            try {
                doc["first"]["fourth"];
            } catch (YAML::KeyNotFound ex) {
                ASSERT_EQ(ex.mark.pos, 10);
                ASSERT_EQ(ex.mark.line, 1);
                ASSERT_EQ(ex.mark.column, 3);
            }

            // mark should be (0,0,0) since doc has no "main2"
            try {
                doc["main2"]["two"];
            } catch (YAML::KeyNotFound ex) {
                ASSERT_EQ(ex.mark.pos, 0);
                ASSERT_EQ(ex.mark.line, 0);
                ASSERT_EQ(ex.mark.column, 0);
            }

            // check marks on "main2" from "main"
            try {
                doc["main"]["main2"]["two"];
            } catch (YAML::KeyNotFound ex) {
                ASSERT_EQ(ex.mark.pos, 54);
                ASSERT_EQ(ex.mark.line, 5);
                ASSERT_EQ(ex.mark.column, 6);
            }
        }
    }  // namespace
}  // namespace YAML
