#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "response-parser.h"

static void print_depth_shift(int depth)
{
    int j;
    for (j = 0; j < depth; j++)
    {
        printf(" ");
    }
}

static void process_value(json_value *value, int depth);

union {
    struct
    {
        unsigned int is_mode_id : 1;
        unsigned int is_direction : 1;
        unsigned int is_red_duration : 1;
        unsigned int is_yellow_duration : 1;
        unsigned int is_green_duration : 1;
    };
    unsigned int flags;
} update_modes;


void update_modes_field(char *field)
{
    update_modes.flags = 0;
    if (strcmp("request_id", field) == 0)
    {
        update_modes.is_mode_id = 1;
    }
    else if (strcmp("direction_1", field) == 0)
    {
        traffic_light_mode.direction = 1;
        printf("updating configuration for direction 1\n");
    }
    else if (strcmp("direction_2", field) == 0)
    {
        traffic_light_mode.direction = 2;
        printf("updating configuration for direction 2\n");
    }
    else if (strcmp("red_duration_sec", field) == 0)
    {
        update_modes.is_red_duration = 1;
    }
    else if (strcmp("yellow_duration_sec", field) == 0)
    {
        update_modes.is_yellow_duration = 1;
    }
    else if (strcmp("green_duration_sec", field) == 0)
    {
        update_modes.is_green_duration = 1;
    }
}

void update_field_values_string(char *value)
{
    if (update_modes.is_mode_id)
    {
        strncpy(traffic_light_mode.mode_id, value, MAX_MODE_LEN);
        printf("updated mode id: %s\n", traffic_light_mode.mode_id);
    }
}

void update_field_values_int(int value)
{
    if (update_modes.is_red_duration)
    {
        traffic_light_mode.red_duration = value;
        printf("updated red duration to %d\n", traffic_light_mode.red_duration);
    }
    else if (update_modes.is_yellow_duration)
    {
        traffic_light_mode.yellow_duration = value;
        printf("updated yellow duration to %d\n", traffic_light_mode.yellow_duration);
    }
    else if (update_modes.is_green_duration)
    {
        traffic_light_mode.green_duration = value;
        printf("updated green duration to %d\n", traffic_light_mode.green_duration);
    }
}

static void process_object(json_value *value, int depth)
{
    int length, x;
    if (value == NULL)
    {
        return;
    }
    length = value->u.object.length;
    for (x = 0; x < length; x++)
    {
        print_depth_shift(depth);
        D(printf("object[%d].name = %s\n", x, value->u.object.values[x].name);)
        update_modes_field(value->u.object.values[x].name);
        process_value(value->u.object.values[x].value, depth + 1);
    }
}

static void process_array(json_value *value, int depth)
{
    int length, x;
    if (value == NULL)
    {
        return;
    }
    length = value->u.array.length;
    D(printf("array\n");)
    for (x = 0; x < length; x++)
    {
        process_value(value->u.array.values[x], depth);
    }
}

static void process_value(json_value *value, int depth)
{
    if (value == NULL)
    {
        return;
    }
    if (value->type != json_object)
    {
        print_depth_shift(depth);
    }
    switch (value->type)
    {
    case json_none:
        D(printf("none\n");)
        break;
    case json_null:
        D(printf("null\n");)
        break;
    case json_object:
        process_object(value, depth + 1);
        break;
    case json_array:
        process_array(value, depth + 1);
        break;
    case json_integer:
        D(printf("int: %10ld\n", (long)value->u.integer);)
        update_field_values_int((int)value->u.integer);
        break;
    case json_double:
        D(printf("double: %f\n", value->u.dbl);)
        break;
    case json_string:
        D(printf("string: %s\n", value->u.string.ptr);)
        update_field_values_string((char *)value->u.string.ptr);
        break;
    case json_boolean:
        D(printf("bool: %d\n", value->u.boolean);)
        break;
    }
}

int parse_response(char *response)
{
    char *filename;
    FILE *fp;
    struct stat filestatus;
    int data_size;
    json_char *json;
    json_value *value;

    // strip everything before the first opening bracket "{"
    data_size = strlen(response);
    for (int i = 0; i < data_size; i++)
    {
        if (response[i] == "{"[0])
        {
            int json_length = 0;
            for (int j = 0; j < data_size - i; j++, json_length++)
            {
                response[j] = response[i + j];
            }
            response[json_length] = 0;
            break;
        }
    }

    printf("--------------------------------\n\n");

    json = (json_char *)response;

    value = json_parse(json, data_size);

    if (value == NULL)
    {
        fprintf(stderr, "Unable to parse data\n");
        free(response);
        exit(1);
    }

    process_value(value, 0);

    json_value_free(value);
    return 0;
}
