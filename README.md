# **dijson** - A tiny, modern, and safe json library
<div align="center">

  <a title="'Build' workflow Status" href="https://github.com/diegomrno/dijson/actions/workflows/build.yml"><img alt="Build" src="https://img.shields.io/github/actions/workflow/status/diegomrno/dijson/build.yml?branch=main&label=Build&style=for-the-badge&logo=GitHub%20Actions&logoColor=fff"></a>
  <a title="Latest Release" href="https://github.com/diegomrno/dijson/releases/latest"><img alt="Latest Release" src="https://img.shields.io/github/v/release/diegomrno/dijson?style=for-the-badge&label=Release&logo=github"></a>
  <a title="Changelog" href="https://github.com/diegomrno/dijson/blob/main/CHANGELOG.md"><img alt="Changelog" src="https://img.shields.io/badge/Changelog-View-blue?style=for-the-badge&logo=readme&logoColor=white"></a>
</div>

> Work in progress...

> **Why this project**: Like [**dilog**](https://github.com/diegomrno/dilog). Because I personally needed a really simple json parsing/dump system tailored to my needs and I didn’t want to much features that I'll never use.

## **Usage**

To use `dilog`, simply include the `dilog.h` header file in your project. No external dependencies are required.

### Hello world with dilog 
```c
#include "../dijson.h"

int main(void) {
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

```
(more doc incoming)

## **Features**

- 📜 **JSON Parsing**: Effortlessly parse a JSON string into an easy-to-use JSON object, with full support for various data types like strings, numbers, booleans, arrays, and objects.
- 📂 **Parse from File**: Load and parse JSON data directly from a file, simplifying the process of handling large datasets stored externally.
- 🔄 **JSON Dump**: Convert a JSON object back into a formatted JSON string, making it simple to inspect or output the data in a readable format.
- 💾 **Dump to File**: Save the generated JSON string to a file, ensuring your JSON data is preserved for future use or sharing.
