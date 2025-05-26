#include <stdio.h>
#include <raylib.h>
#include <math.h>
#include <raymath.h>
#include <string.h>
#include <stdlib.h>

#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 1000

#define MAX_INPUT_CHARS 4
#define INPUT_FIELD_WIDTH 80
#define INPUT_FIELD_HEIGHT 40

// octagon
#define OCTAGON_SIDES 8
#define OCTAGON_RADIUS 450.0f
#define OCTAGON_ROTATION 0.0f
#define OCTAGON_THICKNESS 2.5f

// --------------------------------------

// rhombus
#define RHOMBUS_WIDTH 850.0f
#define RHOMBUS_HEIGHT 850.0f
#define RHOMBUS_THICKNESS 2.5f

// --------------------------------------

//rectangle
#define RECTANGLE_THICKNESS 2.5f
#define RECTANGLE_WIDTH 600
#define RECTANGLE_HEIGHT 600

// --------------------------------------

// circles
// primary
#define BIG_CIRCLE_TOP_HEIGHT 80
#define BIG_CIRCLE_RADIUS 40.0f
#define BIG_CIRCLE_THICKNESS 8.0f

#define MEDIUM_CIRCLE_TOP_HEIGHT 150
#define MEDIUM_CIRCLE_RADIUS 35.0f
#define MEDIUM_CIRCLE_THICKNESS 3.0f

#define SMALL_CIRCLE_TOP_HEIGHT 200
#define SMALL_CIRCLE_RADIUS 25.0f
#define SMALL_CIRCLE_THICKNESS 3.0f

// secondary
#define CIRCLE_THICKNESS 3.0f

//----------------------------------------

typedef enum {
    STATE_INPUT_FORM,
    STATE_MATRIX_VIEW
} AppState;

typedef struct {
    Font font;
    Vector2 position;
    Vector2 origin;
    float rotation;
    float font_size;
    float spacing;
    Color color;
} Text;

typedef struct {
    int day;
    int month;
    int year;
    bool is_valid;
} DateOfBirth;

typedef struct {
    char text[MAX_INPUT_CHARS + 1];
    Rectangle box;
    bool mouse_on_text;
    bool active;
    int letter_count;
    int max_length;
} InputField;

//----------------------------------------

// circles inner/outer radius 
// big
const float inner_radius_big = BIG_CIRCLE_RADIUS - BIG_CIRCLE_THICKNESS / 2;
const float outer_radius_big = BIG_CIRCLE_RADIUS + BIG_CIRCLE_THICKNESS / 2;
// medium
const float inner_radius_medium = MEDIUM_CIRCLE_RADIUS - MEDIUM_CIRCLE_THICKNESS / 2;
const float outer_radius_medium = MEDIUM_CIRCLE_RADIUS + MEDIUM_CIRCLE_THICKNESS / 2;
// small
const float inner_radius_small = SMALL_CIRCLE_RADIUS - SMALL_CIRCLE_THICKNESS / 2;
const float outer_radius_small = SMALL_CIRCLE_RADIUS + SMALL_CIRCLE_THICKNESS / 2;
// other circles
const float inner_radius_big_other = BIG_CIRCLE_RADIUS - CIRCLE_THICKNESS / 2;
const float outer_radius_big_other = BIG_CIRCLE_RADIUS + CIRCLE_THICKNESS / 2;
const float inner_radius_medium_other = MEDIUM_CIRCLE_RADIUS - CIRCLE_THICKNESS / 2;
const float outer_radius_medium_other = MEDIUM_CIRCLE_RADIUS + CIRCLE_THICKNESS / 2;
const float inner_radius_small_other = SMALL_CIRCLE_RADIUS - CIRCLE_THICKNESS / 2;
const float outer_radius_small_other = SMALL_CIRCLE_RADIUS + CIRCLE_THICKNESS / 2;
// matrix-core-circle
const float inner_radius_core_circle = 300.0f - 2.5f / 2;
const float outer_radius_core_circle = 300.0f + 2.5f / 2;

// dashed-line
const Vector2 p1 = {800, 500};
const Vector2 p2 = {500, 800};

//----------------------------------------

void init_input_field(InputField* field, Rectangle box, int max_length) {
    strcpy(field->text, "\0");
    field->box = box;
    field->mouse_on_text = false;
    field->active = false;
    field->letter_count = 0;
    field->max_length = max_length;
}

void update_input_field(InputField* field) {
    if (CheckCollisionPointRec(GetMousePosition(), field->box)) {
        field->mouse_on_text = true;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            field->active = true;
        }
    } else {
        field->mouse_on_text = false;
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            field->active = false;
        }
    }

    if (field->active) {
        int key = GetCharPressed();

        while (key > 0) {
            if ((key >= 48) && (key <= 57) && (field->letter_count < field->max_length)) {
                field->text[field->letter_count] = (char)key;
                field->text[field->letter_count + 1] = '\0';
                field->letter_count++;
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            field->letter_count--;
            if (field->letter_count < 0) field->letter_count = 0;
            field->text[field->letter_count] = '\0';
        }
    }
}

void draw_input_field(InputField* field, Font font, const char* placeholder) {
    // border color
    Color border_color = LIGHTGRAY;
    if (field->mouse_on_text) border_color = DARKBLUE;
    else if (field->active) border_color = BLUE;

    // draw box
    DrawRectangleRec(field->box, RAYWHITE);
    DrawRectangleLinesEx(field->box, 2, border_color);

    // draw text or placeholder
    if (field->letter_count > 0) {
        DrawTextEx(font, field->text, (Vector2){field->box.x + 10, field->box.y + 10}, 20, 2, BLACK);
    } else if (!field->active) {
        DrawTextEx(font, placeholder, (Vector2){field->box.x + 10, field->box.y + 10}, 16, 2, LIGHTGRAY);
    }

    if (field->active) {
        if (((GetTime() * 2) - (int)(GetTime() * 2)) > 0.5f) {
            Vector2 text_size = MeasureTextEx(font, field->text, 20, 2);
            DrawTextEx(font, "_", (Vector2){field->box.x +10 + text_size.x, field->box.y + 10}, 20, 2, BLACK);
        }
    }

}

bool is_valid_date(int day, int month, int year) {
    if (year < 1900 || year > 2025) return false;
    if (month < 1 || month > 12) return false;
    if (day < 1) return false;
    
    int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    // control leap year
    if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) {
        days_in_month[1] = 29;
    }
    
    return day <= days_in_month[month - 1];
}

// struct to contain destiny matrix numbers 
typedef struct {
    // primary numbers
    int big_left;
    int big_top_left;
    int big_top;
    int big_top_right;
    int big_right;
    int big_bottom_right;
    int big_bottom;
    int big_bottom_left;

    // central number
    int center;
    int center_right;

    // secondary numbers
    int medium_left;
    int medium_top_left;
    int medium_top;
    int medium_top_right;
    int medium_right;
    int medium_bottom_right;
    int medium_bottom;
    int medium_bottom_left;

    // tertiary numbers
    int small_left;
    int small_top_left;
    int small_top;
    int small_top_right;
    int small_right;
    int small_bottom_right;
    int small_bottom;
    int small_bottom_left;

    // other
    int money;
    int love;
    int center_bottom;
} DestinyMatrix;

int reduce_to_destiny_number(int number) {
    while (number > 22) {
        int temp = 0; 
        while (number > 0) {
            temp += number % 10;
            number /= 10;
        }
        number = temp;
    }
    return (number == 0) ? 22 : number;
}

DestinyMatrix calculate_destiny_matrix(DateOfBirth dob) {
    DestinyMatrix matrix = {0};
    
    int y1 = (dob.year / 1000) % 10;
    int y2 = (dob.year / 100) % 10;
    int y3 = (dob.year / 10) % 10;
    int y4 = dob.year % 10;
    
    int day_sum = dob.day;
    int month_sum = dob.month;
    int year_sum = y1 + y2 + y3 + y4;

    matrix.big_left = reduce_to_destiny_number(day_sum);
    matrix.big_top = reduce_to_destiny_number(month_sum);
    matrix.big_right = reduce_to_destiny_number(year_sum);
    matrix.big_bottom = reduce_to_destiny_number(matrix.big_left + matrix.big_top + matrix.big_right);

    matrix.center = reduce_to_destiny_number(matrix.big_left + matrix.big_top + matrix.big_right + matrix.big_bottom);

    matrix.big_top_left = reduce_to_destiny_number(matrix.big_left + matrix.big_top);
    matrix.big_top_right = reduce_to_destiny_number(matrix.big_top + matrix.big_right);
    matrix.big_bottom_right = reduce_to_destiny_number(matrix.big_right + matrix.big_bottom);
    matrix.big_bottom_left = reduce_to_destiny_number(matrix.big_bottom + matrix.big_left);

    matrix.small_left = reduce_to_destiny_number(matrix.big_left + matrix.center);
    matrix.small_top = reduce_to_destiny_number(matrix.big_top + matrix.center);
    matrix.small_right = reduce_to_destiny_number(matrix.big_right + matrix.center);
    matrix.small_bottom = reduce_to_destiny_number(matrix.big_bottom + matrix.center);

    matrix.medium_left = reduce_to_destiny_number(matrix.big_left + matrix.small_left);
    matrix.medium_top = reduce_to_destiny_number(matrix.big_top + matrix.small_top);
    matrix.medium_right = reduce_to_destiny_number(matrix.big_right + matrix.small_right);
    matrix.medium_bottom = reduce_to_destiny_number(matrix.big_bottom + matrix.small_bottom);

    matrix.center_right = reduce_to_destiny_number(matrix.big_top_left + matrix.big_top_right + matrix.big_bottom_right + matrix.big_bottom_left);
    
    matrix.small_top_left = reduce_to_destiny_number(matrix.center_right + matrix.big_top_left);
    matrix.small_top_right = reduce_to_destiny_number(matrix.center_right + matrix.big_top_right);
    matrix.small_bottom_right = reduce_to_destiny_number(matrix.center_right + matrix.big_bottom_right);
    matrix.small_bottom_left = reduce_to_destiny_number(matrix.center_right + matrix.big_bottom_left);

    matrix.medium_top_left = reduce_to_destiny_number(matrix.big_top_left + matrix.small_top_left);
    matrix.medium_top_right = reduce_to_destiny_number(matrix.big_top_right + matrix.small_top_right);
    matrix.medium_bottom_right = reduce_to_destiny_number(matrix.big_bottom_left + matrix.small_bottom_right);
    matrix.medium_bottom_left = reduce_to_destiny_number(matrix.big_bottom_left + matrix.small_bottom_left);

    matrix.center_bottom = reduce_to_destiny_number(matrix.small_right + matrix.small_bottom);
    matrix.money = reduce_to_destiny_number(matrix.small_right + matrix.center_bottom);
    matrix.love = reduce_to_destiny_number(matrix.small_bottom + matrix.center_bottom);

    return matrix;
}

//draw functions
void draw_matrix_octagon(Vector2 center) {
    DrawPolyLinesEx(center, OCTAGON_SIDES, OCTAGON_RADIUS, OCTAGON_ROTATION, OCTAGON_THICKNESS, BLACK);
}

void draw_matrix_rhombus(Vector2 center, float width, float height, Color color) {
    Vector2 top    = { center.x, center.y - height / 2 };
    Vector2 right  = { center.x + width / 2, center.y };
    Vector2 bottom = { center.x, center.y + height / 2 };
    Vector2 left   = { center.x - width / 2, center.y };

    DrawLineEx(top, right, RHOMBUS_THICKNESS, color);
    DrawLineEx(right, bottom, RHOMBUS_THICKNESS, color);
    DrawLineEx(bottom, left, RHOMBUS_THICKNESS, color);
    DrawLineEx(left, top, RHOMBUS_THICKNESS, color);
}

void draw_matrix_square(Vector2 center) {
    Rectangle rec = { center.x - 300, center.y - 300, RECTANGLE_WIDTH, RECTANGLE_HEIGHT };
    DrawRectangleLinesEx(rec, RECTANGLE_THICKNESS, BLACK);
}

void draw_matrix_core_circle(Vector2 center) {
    DrawRing(center, inner_radius_core_circle, outer_radius_core_circle, 0, 360, 64, BLACK);
}

void draw_ringed_circle(Vector2 position, float radius, float inner_radius, float outer_radius, Color ring_color) {
    DrawCircleV(position, radius, WHITE);
    DrawRing(position, inner_radius, outer_radius, 0, 360, 64, ring_color);
}

void draw_matrix_circles(Vector2 center) {
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();

    // top
    draw_ringed_circle((Vector2){center.x, screen_height * 0.20f}, SMALL_CIRCLE_RADIUS, inner_radius_small, outer_radius_small, LIGHTGRAY);
    draw_ringed_circle((Vector2){center.x, screen_height * 0.14f}, MEDIUM_CIRCLE_RADIUS, inner_radius_medium, outer_radius_medium, DARKBLUE);
    draw_ringed_circle((Vector2){center.x, screen_height * 0.07f}, BIG_CIRCLE_RADIUS, inner_radius_big, outer_radius_big, GRAY);

    // top-right
    draw_ringed_circle((Vector2){screen_width * 0.71f, screen_height * 0.29f - 5.0f}, SMALL_CIRCLE_RADIUS, inner_radius_small_other, outer_radius_small_other, BLACK);
    draw_ringed_circle((Vector2){screen_width * 0.74f + 15.0f, screen_height * 0.26f - 15.0f}, MEDIUM_CIRCLE_RADIUS, inner_radius_medium_other, outer_radius_medium_other, BLACK);
    draw_ringed_circle((Vector2){screen_width * 0.80f + 10.0f, screen_height * 0.20f - 10.0f}, BIG_CIRCLE_RADIUS, inner_radius_big_other, outer_radius_big_other, BLACK);

    // right
    draw_ringed_circle((Vector2){screen_width * 0.80f, screen_height * 0.50f}, SMALL_CIRCLE_RADIUS, inner_radius_small, outer_radius_small, ORANGE);
    draw_ringed_circle((Vector2){screen_width * 0.86f, screen_height * 0.50f}, MEDIUM_CIRCLE_RADIUS, inner_radius_medium, outer_radius_medium, BLACK);
    draw_ringed_circle((Vector2){screen_width * 0.93f + 2.0f, screen_height * 0.50f}, BIG_CIRCLE_RADIUS, inner_radius_big, outer_radius_big, RED);

    // bottom-right
    draw_ringed_circle((Vector2){screen_width * 0.71f, screen_height * 0.71f}, SMALL_CIRCLE_RADIUS, inner_radius_small_other, outer_radius_small_other, BLACK);
    draw_ringed_circle((Vector2){screen_width * 0.75f + 4.0f, screen_height * 0.75f + 4.0f}, MEDIUM_CIRCLE_RADIUS, inner_radius_medium_other, outer_radius_medium_other, BLACK);
    draw_ringed_circle((Vector2){screen_width * 0.80f + 8.0f, screen_height * 0.80f + 8.0f}, BIG_CIRCLE_RADIUS, inner_radius_big_other, outer_radius_big_other, BLACK);

    // bottom
    draw_ringed_circle((Vector2){center.x, screen_height * 0.80f}, SMALL_CIRCLE_RADIUS, inner_radius_small, outer_radius_small, ORANGE);
    draw_ringed_circle((Vector2){center.x, screen_height * 0.86f}, MEDIUM_CIRCLE_RADIUS, inner_radius_medium, outer_radius_medium, BLACK);
    draw_ringed_circle((Vector2){center.x, screen_height * 0.93f}, BIG_CIRCLE_RADIUS, inner_radius_big, outer_radius_big, RED);

    // bottom-left
    draw_ringed_circle((Vector2){screen_width * 0.29f, screen_height * 0.71f}, SMALL_CIRCLE_RADIUS, inner_radius_small_other, outer_radius_small_other, BLACK);
    draw_ringed_circle((Vector2){screen_width * 0.25f - 4.0f, screen_height * 0.75f + 4.0f}, MEDIUM_CIRCLE_RADIUS, inner_radius_medium_other, outer_radius_medium_other, BLACK);
    draw_ringed_circle((Vector2){screen_width * 0.20f - 8.0f, screen_height * 0.80f + 8.0f}, BIG_CIRCLE_RADIUS, inner_radius_big_other, outer_radius_big_other, BLACK);

    // left
    draw_ringed_circle((Vector2){screen_width * 0.20f, screen_height * 0.50f}, SMALL_CIRCLE_RADIUS, inner_radius_small, outer_radius_small, LIGHTGRAY);
    draw_ringed_circle((Vector2){screen_width * 0.14f, screen_height * 0.50f}, MEDIUM_CIRCLE_RADIUS, inner_radius_medium, outer_radius_medium, DARKBLUE);
    draw_ringed_circle((Vector2){screen_width * 0.07f, screen_height * 0.50f}, BIG_CIRCLE_RADIUS, inner_radius_big, outer_radius_big, GRAY);

    // top-left
    draw_ringed_circle((Vector2){screen_width * 0.29f, screen_height * 0.29f - 5.0f}, SMALL_CIRCLE_RADIUS, inner_radius_small_other, outer_radius_small_other, BLACK);
    draw_ringed_circle((Vector2){screen_width * 0.24f + 3.0f, screen_height * 0.26f - 15.0f}, MEDIUM_CIRCLE_RADIUS, inner_radius_medium_other, outer_radius_medium_other, BLACK);
    draw_ringed_circle((Vector2){screen_width * 0.18f + 10.0f, screen_height * 0.20f - 10.0f}, BIG_CIRCLE_RADIUS, inner_radius_big_other, outer_radius_big_other, BLACK);

    // core
    draw_ringed_circle((Vector2){center.x, center.y}, BIG_CIRCLE_RADIUS, inner_radius_big, outer_radius_big, PURPLE);

    // other
    draw_ringed_circle((Vector2){screen_width * 0.73f, screen_height * 0.57f}, SMALL_CIRCLE_RADIUS, inner_radius_small_other, outer_radius_small_other, BLACK); // money
    draw_ringed_circle((Vector2){screen_width * 0.57f, screen_height * 0.73f}, SMALL_CIRCLE_RADIUS, inner_radius_small_other, outer_radius_small_other, BLACK); // love
    draw_ringed_circle((Vector2){screen_width * 0.65f, screen_height * 0.65f}, SMALL_CIRCLE_RADIUS, inner_radius_small_other, outer_radius_small_other, BLACK); // center-bottom
    draw_ringed_circle((Vector2){screen_width * 0.57f, screen_height * 0.50f}, SMALL_CIRCLE_RADIUS, inner_radius_small_other, outer_radius_small_other, BLACK); // center-right
}   

void draw_matrix_lines(Vector2 center) {
    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    Vector2 start = {center.x, center.y};
    const float thickness = 2.0f;

    // top
    Vector2 end_top = {center.x, 100.0f};
    DrawLineEx(start, end_top, thickness, GRAY);

    // top-right
    Vector2 end_top_right = {screen_width * 0.70f, screen_height * 0.30f};
    DrawLineEx(start, end_top_right, thickness, BLACK);

    // right
    Vector2 end_right = {screen_width * 0.80f, screen_height * 0.50f};
    DrawLineEx(start, end_right, thickness, BLACK);

    // bottom-right
    Vector2 end_bottom_right = {screen_width * 0.70f, screen_height * 0.70f};
    DrawLineEx(start, end_bottom_right, thickness, BLACK);

    // bottom
    Vector2 end_bottom = {screen_width * 0.50f, screen_height * 0.80f};
    DrawLineEx(start, end_bottom, thickness, BLACK);

    // bottom-left
    Vector2 end_bottom_left = {screen_width * 0.30f, screen_height * 0.70f};
    DrawLineEx(start, end_bottom_left, thickness, BLACK);

    // left
    Vector2 end_left = {screen_width * 0.10f, screen_height * 0.50f};
    DrawLineEx(start, end_left, thickness, BLACK);

    // top-left
    Vector2 end_top_left = {screen_width * 0.20f, screen_height * 0.20f};
    DrawLineEx(start, end_top_left, thickness, BLACK);
}

void draw_dashed_line(Vector2 start, Vector2 end, float dash_length, float gap_length, float thickness, Color color) {
    Vector2 direction = Vector2Subtract(end, start);
    float total_length = Vector2Length(direction);
    direction = Vector2Normalize(direction);

    float progress = 0.0f;

    while (progress < total_length) {
        Vector2 dash_start = Vector2Add(start, Vector2Scale(direction, progress));
        float segment_length = fminf(dash_length, total_length - progress);
        Vector2 dash_end = Vector2Add(dash_start, Vector2Scale(direction, segment_length));

        DrawLineEx(dash_start, dash_end, thickness, color);

        progress += dash_length + gap_length;
    }
}

void draw_text(Font font) {
    Text female_text = {font, {530, 430}, {0, 0}, -44.0f, 15.0f, 2.0f, BLACK};
    DrawTextPro(female_text.font, "female generation line", female_text.position, female_text.origin, female_text.rotation, female_text.font_size, female_text.spacing, female_text.color);

    Text male_text = {font, {330, 300}, {0, 0}, 44.0f, 15.0f, 2.0f, BLACK};
    DrawTextPro(male_text.font, "male generation line", male_text.position, male_text.origin, male_text.rotation, male_text.font_size, male_text.spacing, male_text.color);
}

void draw_number(Font font, DateOfBirth dob) {
    if (dob.is_valid) {
        DestinyMatrix matrix = calculate_destiny_matrix(dob);
        char number_str[10];

        int offset = 8;
        
        int screen_width = GetScreenWidth();
        int screen_height = GetScreenHeight();
        Vector2 center = {screen_width / 2.0f, screen_height / 2.0f};

        // central number
        sprintf(number_str, "%d", matrix.center);
        Vector2 center_pos = {center.x - offset, center.y - offset};
        DrawTextEx(font, number_str, center_pos, 24, 2, BLACK);

        // -----------------------------------------------------
        // 8 BIG
        // left
        sprintf(number_str, "%d", matrix.big_left);
        Vector2 big_left_pos = {screen_width * 0.07f - offset, screen_height * 0.50f - offset};
        DrawTextEx(font, number_str, big_left_pos, 24, 2, BLACK);

        // top-left
        sprintf(number_str, "%d", matrix.big_top_left);
        Vector2 big_top_left_pos = {screen_width * 0.18f + 10.0f - offset, screen_height * 0.20f - 10.0f - offset};
        DrawTextEx(font, number_str, big_top_left_pos, 24, 2, BLACK);

        // top
        sprintf(number_str, "%d", matrix.big_top);
        Vector2 big_top_pos = {center.x - offset, screen_height * 0.07f - offset};
        DrawTextEx(font, number_str, big_top_pos, 24, 2, BLACK);

        // top-right
        sprintf(number_str, "%d", matrix.big_top_right);
        Vector2 big_top_right_pos = {screen_width * 0.80f + 10.0f - offset, screen_height * 0.20f - 10.0f - offset};
        DrawTextEx(font, number_str, big_top_right_pos, 24, 2, BLACK);

        // right
        sprintf(number_str, "%d", matrix.big_right);
        Vector2 big_right_pos = {screen_width * 0.93f + 2.0f - offset, screen_height * 0.50f - offset};
        DrawTextEx(font, number_str, big_right_pos, 24, 2, BLACK);

        // bottom-right
        sprintf(number_str, "%d", matrix.big_bottom_right);
        Vector2 big_bottom_right_pos = {screen_width * 0.80f + 8.0f - offset, screen_height * 0.80f + 8.0f- offset};
        DrawTextEx(font, number_str, big_bottom_right_pos, 24, 2, BLACK);

        // bottom
        sprintf(number_str, "%d", matrix.big_bottom);
        Vector2 big_bottom_pos = {center.x - offset, screen_height * 0.93f - offset};
        DrawTextEx(font, number_str, big_bottom_pos, 24, 2, BLACK);

        // bottom-left
        sprintf(number_str, "%d", matrix.big_bottom_left);
        Vector2 big_bottom_left_pos = {screen_width * 0.20f - 8.0f - offset, screen_height * 0.80f + 8.0f - offset};
        DrawTextEx(font, number_str, big_bottom_left_pos, 24, 2, BLACK);

        // -----------------------------------------------------
        // 8 MEDIUM
        // left
        sprintf(number_str, "%d", matrix.medium_left);
        Vector2 medium_left_pos = {screen_width * 0.14f - offset, screen_height * 0.50f - offset};
        DrawTextEx(font, number_str, medium_left_pos, 24, 2, BLACK);

        // top-left
        sprintf(number_str, "%d", matrix.medium_top_left);
        Vector2 medium_top_left_pos = {screen_width * 0.24f + 3.0f - offset, screen_height * 0.26f - 15.0f - offset};
        DrawTextEx(font, number_str, medium_top_left_pos, 24, 2, BLACK);

        // top
        sprintf(number_str, "%d", matrix.medium_top);
        Vector2 medium_top_pos = {center.x - offset, screen_height * 0.14f - offset};
        DrawTextEx(font, number_str, medium_top_pos, 24, 2, BLACK);

        // top-right
        sprintf(number_str, "%d", matrix.medium_top_right);
        Vector2 medium_top_right_pos = {screen_width * 0.74f + 15.0f - offset, screen_height * 0.26f - 15.0f - offset};
        DrawTextEx(font, number_str, medium_top_right_pos, 24, 2, BLACK);

        // right
        sprintf(number_str, "%d", matrix.medium_right);
        Vector2 medium_right_pos = {screen_width * 0.86f - offset, screen_height * 0.50f - offset};
        DrawTextEx(font, number_str, medium_right_pos, 24, 2, BLACK);

        // bottom-right
        sprintf(number_str, "%d", matrix.medium_bottom_right);
        Vector2 medium_bottom_right_pos = {screen_width * 0.75f + 4.0f - offset, screen_height * 0.75f + 4.0f - offset};
        DrawTextEx(font, number_str, medium_bottom_right_pos, 24, 2, BLACK);

        // bottom
        sprintf(number_str, "%d", matrix.medium_bottom);
        Vector2 medium_bottom_pos = {center.x - offset, screen_height * 0.86f - offset};
        DrawTextEx(font, number_str, medium_bottom_pos, 24, 2, BLACK);

        // bottom-left
        sprintf(number_str, "%d", matrix.medium_bottom_left);
        Vector2 medium_bottom_left_pos = {screen_width * 0.25f - 4.0f - offset, screen_height * 0.75f + 4.0f - offset};
        DrawTextEx(font, number_str, medium_bottom_left_pos, 24, 2, BLACK);

        // -----------------------------------------------------
        // 8 SMALL
        // left
        sprintf(number_str, "%d", matrix.small_left);
        Vector2 small_left_pos = {screen_width * 0.20f - offset, screen_height * 0.50f - offset};
        DrawTextEx(font, number_str, small_left_pos, 24, 2, BLACK);

        // top-left
        sprintf(number_str, "%d", matrix.small_top_left);
        Vector2 small_top_left_pos = {screen_width * 0.29f - offset, screen_height * 0.29f - 5.0f - offset};
        DrawTextEx(font, number_str, small_top_left_pos, 24, 2, BLACK);

        // top
        sprintf(number_str, "%d", matrix.small_top);
        Vector2 small_top_pos = {center.x - offset, screen_height * 0.20f - offset};
        DrawTextEx(font, number_str, small_top_pos, 24, 2, BLACK);

        // top-right
        sprintf(number_str, "%d", matrix.small_top_right);
        Vector2 small_top_right_pos = {screen_width * 0.71f - offset, screen_height * 0.29f - 5.0f - offset};
        DrawTextEx(font, number_str, small_top_right_pos, 24, 2, BLACK);

        // right
        sprintf(number_str, "%d", matrix.small_right);
        Vector2 small_right_pos = {screen_width * 0.80f - offset, screen_height * 0.50f - offset};
        DrawTextEx(font, number_str, small_right_pos, 24, 2, BLACK);

        // bottom-right
        sprintf(number_str, "%d", matrix.small_bottom_right);
        Vector2 small_bottom_right_pos = {screen_width * 0.71f - offset, screen_height * 0.71f - offset};
        DrawTextEx(font, number_str, small_bottom_right_pos, 24, 2, BLACK);

        // bottom
        sprintf(number_str, "%d", matrix.small_bottom);
        Vector2 small_bottom_pos = {center.x - offset, screen_height * 0.80f - offset};
        DrawTextEx(font, number_str, small_bottom_pos, 24, 2, BLACK);

        // bottom-left
        sprintf(number_str, "%d", matrix.small_bottom_left);
        Vector2 small_bottom_left_pos = {screen_width * 0.29f - offset, screen_height * 0.71f - offset};
        DrawTextEx(font, number_str, small_bottom_left_pos, 24, 2, BLACK);

        // money
        sprintf(number_str, "%d", matrix.money);
        Vector2 money_pos = {screen_width * 0.73f - offset, screen_height * 0.57f - offset};
        DrawTextEx(font, number_str, money_pos, 24, 2, BLACK);

        // center-bottom
        sprintf(number_str, "%d", matrix.center_bottom);
        Vector2 center_bottom_pos = {screen_width * 0.65f - offset, screen_height * 0.65f - offset};
        DrawTextEx(font, number_str, center_bottom_pos, 24, 2, BLACK);

        // love
        sprintf(number_str, "%d", matrix.love);
        Vector2 love_pos = {screen_width * 0.57f - offset, screen_height * 0.73f - offset};
        DrawTextEx(font, number_str, love_pos, 24, 2, BLACK);

        // center-right
        sprintf(number_str, "%d", matrix.center_right);
        Vector2 center_right_pos = {screen_width * 0.57f - offset, screen_height * 0.50f - offset};
        DrawTextEx(font, number_str, center_right_pos, 24, 2, BLACK);


        // date informations
        char date_str[50];
        sprintf(date_str, "%02d/%02d/%04d", dob.day, dob.month, dob.year);
        Vector2 date_size = MeasureTextEx(font, date_str, 20, 2);
        DrawTextEx(font, date_str, (Vector2){screen_width * 0.09f - date_size.x/2, screen_height * 0.90f}, 20, 2, DARKGRAY);
    }
}

void render_matrix(Vector2 center, Font font, DateOfBirth dob) {
    draw_matrix_octagon(center);
    draw_matrix_rhombus(center, RHOMBUS_WIDTH, RHOMBUS_HEIGHT, BLACK);
    draw_matrix_square(center);
    draw_matrix_core_circle(center);
    draw_matrix_lines(center);
    draw_dashed_line(p1, p2, 10.0f, 5.0f, 2.0f, RED);
    draw_matrix_circles(center);
    draw_text(font);
    draw_number(font, dob);
}

void render_input_form(Font font, InputField* day_field, InputField* month_field, InputField* year_field, 
                      bool show_result, const char* result_text, Color result_color) {
    // background
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(WHITE, 0.9f));
    
    // title
    const char* title = "ENTER YOUR DATE OF BIRTH";
    Vector2 title_size = MeasureTextEx(font, title, 30, 2);
    DrawTextEx(font, title, 
              (Vector2){WINDOW_WIDTH/2 - title_size.x/2, 150}, 
              30, 2, DARKBLUE);
    
    // labels
    DrawTextEx(font, "Day", (Vector2){350, 250}, 20, 2, DARKGRAY);
    DrawTextEx(font, "Month", (Vector2){450, 250}, 20, 2, DARKGRAY);
    DrawTextEx(font, "Year", (Vector2){550, 250}, 20, 2, DARKGRAY);
    
    // input fields
    draw_input_field(day_field, font, "DD");
    draw_input_field(month_field, font, "MM");
    draw_input_field(year_field, font, "YYYY");
    
    // separators
    DrawTextEx(font, "/", (Vector2){435, 290}, 25, 2, DARKGRAY);
    DrawTextEx(font, "/", (Vector2){535, 290}, 25, 2, DARKGRAY);
    
    // instructions
    const char* instructions[] = {
        "Enter your date of birth to generate the Destiny Matrix",
        "Press ENTER to continue",
        "Press ESC to reset"
    };
    
    for (int i = 0; i < 3; i++) {
        Vector2 instr_size = MeasureTextEx(font, instructions[i], 16, 2);
        DrawTextEx(font, instructions[i], 
                  (Vector2){WINDOW_WIDTH/2 - instr_size.x/2, 400 + i * 25}, 
                  16, 2, GRAY);
    }
    
    // result
    if (show_result) {
        Vector2 result_size = MeasureTextEx(font, result_text, 20, 2);
        DrawTextEx(font, result_text, 
                  (Vector2){WINDOW_WIDTH/2 - result_size.x/2, 500}, 
                  20, 2, result_color);
    }
}

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Destiny Matrix");
    SetExitKey(0);
    SetTargetFPS(60);
    Vector2 center = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };

    Font font = LoadFont("./font/Roboto/Roboto-Italic-VariableFont_wdth,wght.ttf");

    AppState current_state = STATE_INPUT_FORM;
    DateOfBirth user_dob = {0, 0, 0, false};

    InputField day_field, month_field, year_field;
    init_input_field(&day_field, (Rectangle){350, 280, INPUT_FIELD_WIDTH, INPUT_FIELD_HEIGHT}, 2);
    init_input_field(&month_field, (Rectangle){450, 280, INPUT_FIELD_WIDTH, INPUT_FIELD_HEIGHT}, 2);
    init_input_field(&year_field, (Rectangle){550, 280, 100, INPUT_FIELD_HEIGHT}, 4);

    bool show_result = false;
    char result_text[100] = "";
    Color result_color = GREEN;

    while (!WindowShouldClose()) {
        if (current_state == STATE_INPUT_FORM) {
            update_input_field(&day_field);
            update_input_field(&month_field);
            update_input_field(&year_field);

            if (IsKeyPressed(KEY_ENTER)) {
                int day = atoi(day_field.text);
                int month = atoi(month_field.text);
                int year = atoi(year_field.text);

                if (strlen(day_field.text) > 0 && strlen(month_field.text) > 0 && strlen(year_field.text) > 0) {
                    if (is_valid_date(day, month, year)) {
                        user_dob.day = day;
                        user_dob.month = month;
                        user_dob.year = year;
                        user_dob.is_valid = true;
                        current_state = STATE_MATRIX_VIEW;
                        show_result = false;
                    } else {
                        sprintf(result_text, "Error: Insert a valid date!");
                        result_color = RED;
                        show_result = true;
                    }
                } else {
                    sprintf(result_text, "You need to fill all fields!");
                    result_color = ORANGE;
                    show_result = true;
                }
            }

            if (IsKeyPressed(KEY_ESCAPE)) {
                strcpy(day_field.text, "");
                strcpy(month_field.text, "");
                strcpy(year_field.text, "");
                day_field.letter_count = 0;
                month_field.letter_count = 0;
                year_field.letter_count = 0;
                show_result = false;
            }
        } else if (current_state == STATE_MATRIX_VIEW) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                current_state = STATE_INPUT_FORM;
            }
        }


        BeginDrawing();
            if (current_state == STATE_MATRIX_VIEW) {
                ClearBackground(WHITE);
                render_matrix(center, font, user_dob);

                DrawTextEx(font, "Press ESC to insert a new date", (Vector2){20, WINDOW_HEIGHT - 30}, 16, 2, DARKGRAY);
            } else {
                ClearBackground(LIGHTGRAY);
                render_input_form(font, &day_field, &month_field, &year_field, show_result, result_text, result_color);
            }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}