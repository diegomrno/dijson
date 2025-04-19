#include "../dijson.h"

int main() {
    const char* json_text = "{ \"name\": \"Me\", \"year\": 2025, \"alive\": true, \"skills\": [\"Fire\", \"Ice\"] }";

    // parse
    dijson_value_t* root = dijson_parse(json_text);
    if (!root) {
        fprintf(stderr, "Failed to parse JSON.\n");
        return 1;
    }
    const char* name = dijson_get_string(dijson_get(root, "name"));
    double year = dijson_get_number(dijson_get(root, "year"));
    bool alive = dijson_get_bool(dijson_get(root, "alive"));

    printf("Name: %s\n", name);
    printf("Year: %.0f\n", year);
    printf("Alive: %s\n", alive ? "true" : "false");

    dijson_value_t* skills = dijson_get(root, "skills");
    if (skills && skills->type == DIJSON_ARRAY) {
        printf("Skills:\n");
        for (size_t i = 0; i < skills->as.array.count; ++i) {
            const char* skill = dijson_get_string(dijson_get_index(skills, i));
            printf("  - %s\n", skill);
        }
    }

    char* dumped = dijson_dump(root);
    printf("\nDumped JSON: %s\n", dumped);
    free(dumped);

    // free
    dijson_free(root);
    return 0;
}