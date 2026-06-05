#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <math.h>

#define WIDTH 80
#define HEIGHT 20
#define MAX_SHAPES 100

// Arrow key codes and specials (mapped from _getch())
#define KEY_UP 1072
#define KEY_DOWN 1080
#define KEY_LEFT 1075
#define KEY_RIGHT 1077
#define KEY_ENTER 13
#define KEY_ESC 27

// Define shape types
typedef enum {
    SHAPE_LINE,
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE
} ShapeType;

// Define shape colors
typedef enum {
    COLOR_DEFAULT,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_YELLOW,
    COLOR_BLUE,
    COLOR_MAGENTA,
    COLOR_CYAN,
    COLOR_WHITE
} ShapeColor;

// Coordinate point
typedef struct {
    int x;
    int y;
} Point;

// Parameter structures for shapes
typedef struct {
    Point start;
    Point end;
} LineParams;

typedef struct {
    Point top_left;
    int width;
    int height;
} RectParams;

typedef struct {
    Point center;
    int radius;
} CircleParams;

typedef struct {
    Point p1;
    Point p2;
    Point p3;
} TriangleParams;

// Unified Shape structure with color and filled state
typedef struct {
    int id;
    ShapeType type;
    union {
        LineParams line;
        RectParams rect;
        CircleParams circle;
        TriangleParams triangle;
    } params;
    char draw_char;
    ShapeColor color;
    int is_filled; // 1 if filled, 0 if outline
    int active;
} Shape;

// Global application state
char canvas[HEIGHT][WIDTH];
unsigned char canvas_color[HEIGHT][WIDTH]; // Parallel color canvas array
char bg_char = '_';
char default_draw_char = '*';
Shape shapes[MAX_SHAPES];
int next_id = 1;

// Clear the canvas to the current background character and default color
void clear_canvas() {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            canvas[y][x] = bg_char;
            canvas_color[y][x] = COLOR_DEFAULT;
        }
    }
}

// Draw a single character safely within bounds with color
void draw_pixel(int x, int y, char ch, ShapeColor color) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        canvas[y][x] = ch;
        canvas_color[y][x] = color;
    }
}

// Bresenham's Line Algorithm
void draw_line(int x0, int y0, int x1, int y1, char ch, ShapeColor color) {
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        draw_pixel(x0, y0, ch, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// Draw Rectangle outline
void draw_rectangle(int x, int y, int w, int h, char ch, ShapeColor color) {
    if (w <= 0 || h <= 0) return;
    // Top and bottom sides
    for (int i = 0; i < w; i++) {
        draw_pixel(x + i, y, ch, color);
        draw_pixel(x + i, y + h - 1, ch, color);
    }
    // Left and right sides
    for (int i = 0; i < h; i++) {
        draw_pixel(x, y + i, ch, color);
        draw_pixel(x + w - 1, y + i, ch, color);
    }
}

// Draw Filled Rectangle
void draw_filled_rectangle(int x, int y, int w, int h, char ch, ShapeColor color) {
    if (w <= 0 || h <= 0) return;
    for (int row = y; row < y + h; row++) {
        for (int col = x; col < x + w; col++) {
            draw_pixel(col, row, ch, color);
        }
    }
}

// Draw horizontal scanline helper (used for filled circles and triangles)
void draw_horizontal_line(int x1, int x2, int y, char ch, ShapeColor color) {
    int start = (x1 < x2) ? x1 : x2;
    int end = (x1 > x2) ? x1 : x2;
    for (int col = start; col <= end; col++) {
        draw_pixel(col, y, ch, color);
    }
}

// Helper to plot 8-way symmetric points for a circle
void plot_circle_points(int xc, int yc, int x, int y, char ch, ShapeColor color) {
    draw_pixel(xc + x, yc + y, ch, color);
    draw_pixel(xc - x, yc + y, ch, color);
    draw_pixel(xc + x, yc - y, ch, color);
    draw_pixel(xc - x, yc - y, ch, color);
    draw_pixel(xc + y, yc + x, ch, color);
    draw_pixel(xc - y, yc + x, ch, color);
    draw_pixel(xc + y, yc - x, ch, color);
    draw_pixel(xc - y, yc - x, ch, color);
}

// Midpoint Circle Algorithm
void draw_circle(int xc, int yc, int r, char ch, ShapeColor color) {
    if (r <= 0) {
        draw_pixel(xc, yc, ch, color);
        return;
    }
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    plot_circle_points(xc, yc, x, y, ch, color);
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        plot_circle_points(xc, yc, x, y, ch, color);
    }
}

// Helper to plot filled circle scanlines
void plot_filled_circle_points(int xc, int yc, int x, int y, char ch, ShapeColor color) {
    draw_horizontal_line(xc - x, xc + x, yc + y, ch, color);
    draw_horizontal_line(xc - x, xc + x, yc - y, ch, color);
    draw_horizontal_line(xc - y, xc + y, yc + x, ch, color);
    draw_horizontal_line(xc - y, xc + y, yc - x, ch, color);
}

// Draw Filled Circle using horizontal scanlines
void draw_filled_circle(int xc, int yc, int r, char ch, ShapeColor color) {
    if (r <= 0) {
        draw_pixel(xc, yc, ch, color);
        return;
    }
    int x = 0;
    int y = r;
    int d = 3 - 2 * r;

    plot_filled_circle_points(xc, yc, x, y, ch, color);
    while (y >= x) {
        x++;
        if (d > 0) {
            y--;
            d = d + 4 * (x - y) + 10;
        } else {
            d = d + 4 * x + 6;
        }
        plot_filled_circle_points(xc, yc, x, y, ch, color);
    }
}

// Draw Triangle outline (lines connecting three vertices)
void draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, char ch, ShapeColor color) {
    draw_line(x1, y1, x2, y2, ch, color);
    draw_line(x2, y2, x3, y3, ch, color);
    draw_line(x3, y3, x1, y1, ch, color);
}

// Scanline helper for flat-bottom triangles
void draw_flat_bottom_triangle(int x1, int y1, int x2, int y2, int x3, int y3, char ch, ShapeColor color) {
    float invslope1 = (float)(x2 - x1) / (y2 - y1);
    float invslope2 = (float)(x3 - x1) / (y3 - y1);

    float curx1 = x1;
    float curx2 = x1;

    for (int scanlineY = y1; scanlineY <= y2; scanlineY++) {
        draw_horizontal_line((int)(curx1 + 0.5f), (int)(curx2 + 0.5f), scanlineY, ch, color);
        curx1 += invslope1;
        curx2 += invslope2;
    }
}

// Scanline helper for flat-top triangles
void draw_flat_top_triangle(int x1, int y1, int x2, int y2, int x3, int y3, char ch, ShapeColor color) {
    float invslope1 = (float)(x3 - x1) / (y3 - y1);
    float invslope2 = (float)(x3 - x2) / (y3 - y2);

    float curx1 = x3;
    float curx2 = x3;

    for (int scanlineY = y3; scanlineY > y1; scanlineY--) {
        curx1 -= invslope1;
        curx2 -= invslope2;
        draw_horizontal_line((int)(curx1 + 0.5f), (int)(curx2 + 0.5f), scanlineY - 1, ch, color);
    }
}

// Draw Filled Triangle using scanline rasterization
void draw_filled_triangle(int x1, int y1, int x2, int y2, int x3, int y3, char ch, ShapeColor color) {
    // Sort vertices by y coordinate (y1 <= y2 <= y3)
    int temp;
    if (y1 > y2) {
        temp = y1; y1 = y2; y2 = temp;
        temp = x1; x1 = x2; x2 = temp;
    }
    if (y1 > y3) {
        temp = y1; y1 = y3; y3 = temp;
        temp = x1; x1 = x3; x3 = temp;
    }
    if (y2 > y3) {
        temp = y2; y2 = y3; y3 = temp;
        temp = x2; x2 = x3; x3 = temp;
    }

    if (y1 == y3) {
        // Degenerate flat horizontal line triangle
        draw_horizontal_line(x1, x2, y1, ch, color);
        draw_horizontal_line(x2, x3, y1, ch, color);
        return;
    }

    if (y2 == y3) {
        draw_flat_bottom_triangle(x1, y1, x2, y2, x3, y3, ch, color);
    } else if (y1 == y2) {
        draw_flat_top_triangle(x1, y1, x2, y2, x3, y3, ch, color);
    } else {
        // General case: split into flat-bottom and flat-top
        int x4 = (int)(x1 + ((float)(y2 - y1) / (y3 - y1)) * (x3 - x1) + 0.5f);
        draw_flat_bottom_triangle(x1, y1, x2, y2, x4, y2, ch, color);
        draw_flat_top_triangle(x2, y2, x4, y2, x3, y3, ch, color);
    }
}

// Render all shapes to the canvas array
void render_all_shapes() {
    clear_canvas();
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active) {
            char ch = shapes[i].draw_char;
            ShapeColor color = shapes[i].color;
            int filled = shapes[i].is_filled;
            switch (shapes[i].type) {
                case SHAPE_LINE:
                    draw_line(shapes[i].params.line.start.x, shapes[i].params.line.start.y,
                              shapes[i].params.line.end.x, shapes[i].params.line.end.y, ch, color);
                    break;
                case SHAPE_RECTANGLE:
                    if (filled) {
                        draw_filled_rectangle(shapes[i].params.rect.top_left.x, shapes[i].params.rect.top_left.y,
                                              shapes[i].params.rect.width, shapes[i].params.rect.height, ch, color);
                    } else {
                        draw_rectangle(shapes[i].params.rect.top_left.x, shapes[i].params.rect.top_left.y,
                                       shapes[i].params.rect.width, shapes[i].params.rect.height, ch, color);
                    }
                    break;
                case SHAPE_CIRCLE:
                    if (filled) {
                        draw_filled_circle(shapes[i].params.circle.center.x, shapes[i].params.circle.center.y,
                                           shapes[i].params.circle.radius, ch, color);
                    } else {
                        draw_circle(shapes[i].params.circle.center.x, shapes[i].params.circle.center.y,
                                    shapes[i].params.circle.radius, ch, color);
                    }
                    break;
                case SHAPE_TRIANGLE:
                    if (filled) {
                        draw_filled_triangle(shapes[i].params.triangle.p1.x, shapes[i].params.triangle.p1.y,
                                             shapes[i].params.triangle.p2.x, shapes[i].params.triangle.p2.y,
                                             shapes[i].params.triangle.p3.x, shapes[i].params.triangle.p3.y, ch, color);
                    } else {
                        draw_triangle(shapes[i].params.triangle.p1.x, shapes[i].params.triangle.p1.y,
                                      shapes[i].params.triangle.p2.x, shapes[i].params.triangle.p2.y,
                                      shapes[i].params.triangle.p3.x, shapes[i].params.triangle.p3.y, ch, color);
                    }
                    break;
            }
        }
    }
}

// Display canvas to console with ANSI colors
void display_canvas() {
    // Print column ruler for reference (tens)
    printf("   ");
    for (int x = 0; x < WIDTH; x++) {
        if (x % 10 == 0) printf("%d", x / 10);
        else printf(" ");
    }
    printf("\n");

    // Print column ruler (units)
    printf("   ");
    for (int x = 0; x < WIDTH; x++) {
        printf("%d", x % 10);
    }
    printf("\n");

    // Top border
    printf("  +");
    for (int x = 0; x < WIDTH; x++) printf("-");
    printf("+\n");

    // Canvas contents with row indices and color mappings
    for (int y = 0; y < HEIGHT; y++) {
        printf("%2d|", y);
        for (int x = 0; x < WIDTH; x++) {
            ShapeColor col = canvas_color[y][x];
            switch (col) {
                case COLOR_RED:     printf("\033[1;31m"); break;
                case COLOR_GREEN:   printf("\033[1;32m"); break;
                case COLOR_YELLOW:  printf("\033[1;33m"); break;
                case COLOR_BLUE:    printf("\033[1;34m"); break;
                case COLOR_MAGENTA: printf("\033[1;35m"); break;
                case COLOR_CYAN:    printf("\033[1;36m"); break;
                case COLOR_WHITE:   printf("\033[1;37m"); break;
                default:            break;
            }
            printf("%c", canvas[y][x]);
            if (col != COLOR_DEFAULT) {
                printf("\033[0m"); // Reset ANSI colors
            }
        }
        printf("|\n");
    }

    // Bottom border
    printf("  +");
    for (int x = 0; x < WIDTH; x++) printf("-");
    printf("+\n");
}

// List all active shapes with details
void list_active_shapes() {
    printf("\n--- Active Shapes List ---\n");
    int count = 0;
    const char* col_names[] = {"Default", "Red", "Green", "Yellow", "Blue", "Magenta", "Cyan", "White"};
    const char* col_escapes[] = {"", "\033[1;31m", "\033[1;32m", "\033[1;33m", "\033[1;34m", "\033[1;35m", "\033[1;36m", "\033[1;37m"};

    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active) {
            count++;
            printf("ID: %-3d | ", shapes[i].id);
            switch (shapes[i].type) {
                case SHAPE_LINE:
                    printf("Line: (%d, %d) to (%d, %d) | Draw Char: '%c' | Color: %s%s\033[0m\n",
                           shapes[i].params.line.start.x, shapes[i].params.line.start.y,
                           shapes[i].params.line.end.x, shapes[i].params.line.end.y,
                           shapes[i].draw_char, col_escapes[shapes[i].color], col_names[shapes[i].color]);
                    break;
                case SHAPE_RECTANGLE:
                    printf("Rectangle: top-left(%d, %d), width: %d, height: %d | Style: %s | Draw Char: '%c' | Color: %s%s\033[0m\n",
                           shapes[i].params.rect.top_left.x, shapes[i].params.rect.top_left.y,
                           shapes[i].params.rect.width, shapes[i].params.rect.height,
                           shapes[i].is_filled ? "Filled" : "Outline",
                           shapes[i].draw_char, col_escapes[shapes[i].color], col_names[shapes[i].color]);
                    break;
                case SHAPE_CIRCLE:
                    printf("Circle: center(%d, %d), radius: %d | Style: %s | Draw Char: '%c' | Color: %s%s\033[0m\n",
                           shapes[i].params.circle.center.x, shapes[i].params.circle.center.y,
                           shapes[i].params.circle.radius,
                           shapes[i].is_filled ? "Filled" : "Outline",
                           shapes[i].draw_char, col_escapes[shapes[i].color], col_names[shapes[i].color]);
                    break;
                case SHAPE_TRIANGLE:
                    printf("Triangle: p1(%d, %d), p2(%d, %d), p3(%d, %d) | Style: %s | Draw Char: '%c' | Color: %s%s\033[0m\n",
                           shapes[i].params.triangle.p1.x, shapes[i].params.triangle.p1.y,
                           shapes[i].params.triangle.p2.x, shapes[i].params.triangle.p2.y,
                           shapes[i].params.triangle.p3.x, shapes[i].params.triangle.p3.y,
                           shapes[i].is_filled ? "Filled" : "Outline",
                           shapes[i].draw_char, col_escapes[shapes[i].color], col_names[shapes[i].color]);
                    break;
            }
        }
    }
    if (count == 0) {
        printf("(No shapes drawn yet)\n");
    }
    printf("---------------------------\n");
}

// Add shape helper
int add_shape(ShapeType type, void* params, char draw_char, ShapeColor color, int is_filled) {
    int index = -1;
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (!shapes[i].active) {
            index = i;
            break;
        }
    }
    if (index == -1) {
        printf("Error: Maximum shape limit (%d) reached!\n", MAX_SHAPES);
        return -1;
    }

    shapes[index].id = next_id++;
    shapes[index].type = type;
    shapes[index].draw_char = draw_char;
    shapes[index].color = color;
    shapes[index].is_filled = is_filled;
    shapes[index].active = 1;

    switch (type) {
        case SHAPE_LINE:
            shapes[index].params.line = *(LineParams*)params;
            break;
        case SHAPE_RECTANGLE:
            shapes[index].params.rect = *(RectParams*)params;
            break;
        case SHAPE_CIRCLE:
            shapes[index].params.circle = *(CircleParams*)params;
            break;
        case SHAPE_TRIANGLE:
            shapes[index].params.triangle = *(TriangleParams*)params;
            break;
    }
    return shapes[index].id;
}

// Delete shape by ID
int delete_shape(int id) {
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active && shapes[i].id == id) {
            shapes[i].active = 0;
            return 1;
        }
    }
    return 0;
}

// Find pointer to shape by ID
Shape* find_shape(int id) {
    for (int i = 0; i < MAX_SHAPES; i++) {
        if (shapes[i].active && shapes[i].id == id) {
            return &shapes[i];
        }
    }
    return NULL;
}

// Safe input reading helper functions
int read_int(const char* prompt, int* value) {
    char buffer[128];
    printf("%s", prompt);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return 0;
    }
    return sscanf(buffer, "%d", value) == 1;
}

int read_char(const char* prompt, char* value) {
    char buffer[128];
    printf("%s", prompt);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return 0;
    }
    int i = 0;
    while (buffer[i] == ' ' || buffer[i] == '\t') i++;
    if (buffer[i] == '\n' || buffer[i] == '\0') {
        return 0;
    }
    *value = buffer[i];
    return 1;
}

// Get raw/special keypresses on Windows
int get_key() {
    int ch = _getch();
    if (ch == 0 || ch == 224) {
        int ch2 = _getch();
        return 1000 + ch2;
    }
    return ch;
}

// Visual cursor position selection on the canvas with shape previews
int select_point_on_canvas(const char* prompt, Point* pt, Point* preview_pts, int preview_count, ShapeType preview_type) {
    int cx = WIDTH / 2;
    int cy = HEIGHT / 2;
    if (pt->x >= 0 && pt->x < WIDTH && pt->y >= 0 && pt->y < HEIGHT) {
        cx = pt->x;
        cy = pt->y;
    }

    while (1) {
        // 1. Render committed shapes
        render_all_shapes();

        // 2. Draw live preview construction lines using '+' (in Cyan)
        if (preview_count > 0) {
            for (int i = 0; i < preview_count; i++) {
                draw_pixel(preview_pts[i].x, preview_pts[i].y, 'O', COLOR_CYAN);
            }

            if (preview_type == SHAPE_LINE && preview_count == 1) {
                draw_line(preview_pts[0].x, preview_pts[0].y, cx, cy, '+', COLOR_CYAN);
            }
            else if (preview_type == SHAPE_RECTANGLE && preview_count == 1) {
                int px = preview_pts[0].x;
                int py = preview_pts[0].y;
                int xmin = (cx < px) ? cx : px;
                int ymin = (cy < py) ? cy : py;
                int w = abs(cx - px) + 1;
                int h = abs(cy - py) + 1;
                draw_rectangle(xmin, ymin, w, h, '+', COLOR_CYAN);
            }
            else if (preview_type == SHAPE_CIRCLE && preview_count == 1) {
                int px = preview_pts[0].x;
                int py = preview_pts[0].y;
                int dx = cx - px;
                int dy = cy - py;
                int r = (int)(sqrt(dx * dx + dy * dy) + 0.5);
                draw_circle(px, py, r, '+', COLOR_CYAN);
            }
            else if (preview_type == SHAPE_TRIANGLE) {
                if (preview_count == 1) {
                    draw_line(preview_pts[0].x, preview_pts[0].y, cx, cy, '+', COLOR_CYAN);
                } else if (preview_count == 2) {
                    draw_line(preview_pts[0].x, preview_pts[0].y, preview_pts[1].x, preview_pts[1].y, '+', COLOR_CYAN);
                    draw_line(preview_pts[1].x, preview_pts[1].y, cx, cy, '+', COLOR_CYAN);
                    draw_line(cx, cy, preview_pts[0].x, preview_pts[0].y, '+', COLOR_CYAN);
                }
            }
        }

        // 3. Temporarily draw the visual cursor as '+' (in Yellow)
        char orig = canvas[cy][cx];
        unsigned char orig_col = canvas_color[cy][cx];
        draw_pixel(cx, cy, '+', COLOR_YELLOW);

        // 4. Redraw screen
        system("cls");
        printf("=================== COORDINATE SELECTOR ===================\n");
        display_canvas();
        printf("%s\n", prompt);
        printf("Current Position: (%d, %d)\n", cx, cy);
        printf("Controls: Arrow Keys to move cursor, Enter to confirm, Esc to cancel.\n");

        // 5. Restore original char & color
        canvas[cy][cx] = orig;
        canvas_color[cy][cx] = orig_col;

        // 6. Read key
        int key = get_key();
        if (key == KEY_UP) {
            cy = (cy - 1 + HEIGHT) % HEIGHT;
        } else if (key == KEY_DOWN) {
            cy = (cy + 1) % HEIGHT;
        } else if (key == KEY_LEFT) {
            cx = (cx - 1 + WIDTH) % WIDTH;
        } else if (key == KEY_RIGHT) {
            cx = (cx + 1) % WIDTH;
        } else if (key == KEY_ENTER) {
            pt->x = cx;
            pt->y = cy;
            return 1;
        } else if (key == KEY_ESC) {
            return 0;
        }
    }
}

// Get standard drawing character (allows hitting enter to accept default/current)
char get_draw_char_prompt(char current) {
    char buffer[128];
    printf("Enter drawing character (current '%c', press Enter to keep): ", current);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return current;
    }
    int i = 0;
    while (buffer[i] == ' ' || buffer[i] == '\t') i++;
    if (buffer[i] == '\n' || buffer[i] == '\0') {
        return current;
    }
    return buffer[i];
}

// Interactive Color Selector using Arrow Keys & Enter
ShapeColor prompt_select_color() {
    const char* color_names[] = {
        "Default / Theme Color",
        "Red",
        "Green",
        "Yellow",
        "Blue",
        "Magenta",
        "Cyan",
        "White"
    };
    const char* color_escapes[] = {
        "\033[0m",      // Default
        "\033[1;31m",   // Red
        "\033[1;32m",   // Green
        "\033[1;33m",   // Yellow
        "\033[1;34m",   // Blue
        "\033[1;35m",   // Magenta
        "\033[1;36m",   // Cyan
        "\033[1;37m"    // White
    };

    int choice = 0;
    while (1) {
        system("cls");
        printf("=================== SELECT COLOR ===================\n");
        for (int i = 0; i < 8; i++) {
            if (i == choice) {
                printf(" -> %s%s\033[0m\n", color_escapes[i], color_names[i]);
            } else {
                printf("    %s%s\033[0m\n", color_escapes[i], color_names[i]);
            }
        }
        printf("====================================================\n");
        printf("Use Arrow Keys (Up/Down) to navigate, Enter to select.\n");

        int key = get_key();
        if (key == KEY_UP) {
            choice = (choice - 1 + 8) % 8;
        } else if (key == KEY_DOWN) {
            choice = (choice + 1) % 8;
        } else if (key == KEY_ENTER) {
            return (ShapeColor)choice;
        }
    }
}

// Interactive Fill Mode Selector using Arrow Keys & Enter
int prompt_select_fill_mode() {
    const char* options[] = {
        "Outline Mode (Wireframe)",
        "Filled Mode (Solid)"
    };

    int choice = 0;
    while (1) {
        system("cls");
        printf("=================== SELECT RENDER STYLE ===================\n");
        for (int i = 0; i < 2; i++) {
            if (i == choice) {
                printf("\033[1;36m -> %s\033[0m\n", options[i]);
            } else {
                printf("    %s\n", options[i]);
            }
        }
        printf("===========================================================\n");
        printf("Use Arrow Keys (Up/Down) to navigate, Enter to select.\n");

        int key = get_key();
        if (key == KEY_UP || key == KEY_DOWN) {
            choice = (choice + 1) % 2;
        } else if (key == KEY_ENTER) {
            return choice; // 0 = Outline, 1 = Filled
        }
    }
}

void prompt_add_shape() {
    const char* add_menu_options[] = {
        "Line",
        "Rectangle",
        "Circle",
        "Triangle",
        "Back to Main Menu"
    };
    
    int choice = 0;
    while (1) {
        system("cls");
        printf("=================== ADD SHAPE ===================\n");
        for (int i = 0; i < 5; i++) {
            if (i == choice) {
                printf("\033[1;36m -> %s\033[0m\n", add_menu_options[i]);
            } else {
                printf("    %s\n", add_menu_options[i]);
            }
        }
        printf("=================================================\n");
        printf("Use Arrow Keys (Up/Down) to navigate, Enter to select.\n");
        
        int key = get_key();
        if (key == KEY_UP) {
            choice = (choice - 1 + 5) % 5;
        } else if (key == KEY_DOWN) {
            choice = (choice + 1) % 5;
        } else if (key == KEY_ENTER) {
            break;
        }
    }
    
    if (choice == 4) return; // Back to main menu
    
    char draw_char = default_draw_char;
    ShapeColor shape_col = COLOR_DEFAULT;
    int is_filled = 0;
    
    if (choice == 0) { // Line
        Point p1 = {WIDTH / 2, HEIGHT / 2};
        Point p2 = {WIDTH / 2, HEIGHT / 2};
        if (!select_point_on_canvas("Select LINE START point and press Enter:", &p1, NULL, 0, SHAPE_LINE)) {
            printf("\nLine creation cancelled. Press any key to continue...");
            _getch();
            return;
        }
        if (!select_point_on_canvas("Select LINE END point and press Enter:", &p2, &p1, 1, SHAPE_LINE)) {
            printf("\nLine creation cancelled. Press any key to continue...");
            _getch();
            return;
        }
        system("cls");
        draw_char = get_draw_char_prompt(default_draw_char);
        shape_col = prompt_select_color();
        LineParams lp = {p1, p2};
        add_shape(SHAPE_LINE, &lp, draw_char, shape_col, 0);
        printf("\nLine added successfully! Press any key to continue...");
        _getch();
    } 
    else if (choice == 1) { // Rectangle
        Point p1 = {WIDTH / 2, HEIGHT / 2};
        Point p2 = {WIDTH / 2, HEIGHT / 2};
        if (!select_point_on_canvas("Select RECTANGLE START corner point and press Enter:", &p1, NULL, 0, SHAPE_RECTANGLE)) {
            printf("\nRectangle creation cancelled. Press any key to continue...");
            _getch();
            return;
        }
        if (!select_point_on_canvas("Select RECTANGLE OPPOSITE corner point and press Enter:", &p2, &p1, 1, SHAPE_RECTANGLE)) {
            printf("\nRectangle creation cancelled. Press any key to continue...");
            _getch();
            return;
        }
        system("cls");
        draw_char = get_draw_char_prompt(default_draw_char);
        shape_col = prompt_select_color();
        is_filled = prompt_select_fill_mode();
        int xmin = (p1.x < p2.x) ? p1.x : p2.x;
        int ymin = (p1.y < p2.y) ? p1.y : p2.y;
        int w = abs(p2.x - p1.x) + 1;
        int h = abs(p2.y - p1.y) + 1;
        RectParams rp = {{xmin, ymin}, w, h};
        add_shape(SHAPE_RECTANGLE, &rp, draw_char, shape_col, is_filled);
        printf("\nRectangle added successfully! Press any key to continue...");
        _getch();
    } 
    else if (choice == 2) { // Circle
        Point center = {WIDTH / 2, HEIGHT / 2};
        Point edge = {WIDTH / 2, HEIGHT / 2};
        if (!select_point_on_canvas("Select CIRCLE CENTER point and press Enter:", &center, NULL, 0, SHAPE_CIRCLE)) {
            printf("\nCircle creation cancelled. Press any key to continue...");
            _getch();
            return;
        }
        if (!select_point_on_canvas("Move cursor to set CIRCLE RADIUS and press Enter:", &edge, &center, 1, SHAPE_CIRCLE)) {
            printf("\nCircle creation cancelled. Press any key to continue...");
            _getch();
            return;
        }
        system("cls");
        draw_char = get_draw_char_prompt(default_draw_char);
        shape_col = prompt_select_color();
        is_filled = prompt_select_fill_mode();
        int dx = edge.x - center.x;
        int dy = edge.y - center.y;
        int r = (int)(sqrt(dx * dx + dy * dy) + 0.5);
        CircleParams cp = {center, r};
        add_shape(SHAPE_CIRCLE, &cp, draw_char, shape_col, is_filled);
        printf("\nCircle added successfully! Press any key to continue...");
        _getch();
    } 
    else if (choice == 3) { // Triangle
        Point p1 = {WIDTH / 2, HEIGHT / 2};
        Point p2 = {WIDTH / 2, HEIGHT / 2};
        Point p3 = {WIDTH / 2, HEIGHT / 2};
        Point preview[2];
        if (!select_point_on_canvas("Select TRIANGLE POINT 1 and press Enter:", &p1, NULL, 0, SHAPE_TRIANGLE)) {
            printf("\nTriangle creation cancelled. Press any key to continue...");
            _getch();
            return;
        }
        preview[0] = p1;
        if (!select_point_on_canvas("Select TRIANGLE POINT 2 and press Enter:", &p2, preview, 1, SHAPE_TRIANGLE)) {
            printf("\nTriangle creation cancelled. Press any key to continue...");
            _getch();
            return;
        }
        preview[1] = p2;
        if (!select_point_on_canvas("Select TRIANGLE POINT 3 and press Enter:", &p3, preview, 2, SHAPE_TRIANGLE)) {
            printf("\nTriangle creation cancelled. Press any key to continue...");
            _getch();
            return;
        }
        system("cls");
        draw_char = get_draw_char_prompt(default_draw_char);
        shape_col = prompt_select_color();
        is_filled = prompt_select_fill_mode();
        TriangleParams tp = {p1, p2, p3};
        add_shape(SHAPE_TRIANGLE, &tp, draw_char, shape_col, is_filled);
        printf("\nTriangle added successfully! Press any key to continue...");
        _getch();
    }
}

void prompt_delete_shape() {
    system("cls");
    printf("=================== DELETE SHAPE ===================\n");
    list_active_shapes();
    
    int id;
    if (!read_int("Enter Shape ID to delete (or press Enter to cancel): ", &id)) {
        return;
    }
    if (delete_shape(id)) {
        printf("\nShape with ID %d deleted successfully. Press any key to continue...", id);
    } else {
        printf("\nShape with ID %d not found. Press any key to continue...", id);
    }
    _getch();
}

void prompt_modify_shape() {
    system("cls");
    printf("=================== MODIFY SHAPE ===================\n");
    list_active_shapes();
    
    int id;
    if (!read_int("Enter Shape ID to modify (or press Enter to cancel): ", &id)) {
        return;
    }

    Shape* sh = find_shape(id);
    if (!sh) {
        printf("\nShape with ID %d not found. Press any key to continue...", id);
        _getch();
        return;
    }

    const char* modify_options[] = {
        "Edit coordinates/dimensions visually",
        "Edit drawing character",
        "Edit shape color",
        "Edit fill style (Outline vs Filled)",
        "Edit all properties",
        "Cancel"
    };

    int choice = 0;
    while (1) {
        system("cls");
        printf("=================== MODIFY SHAPE ID %d ===================\n", id);
        for (int i = 0; i < 6; i++) {
            if (i == choice) {
                printf("\033[1;36m -> %s\033[0m\n", modify_options[i]);
            } else {
                printf("    %s\n", modify_options[i]);
            }
        }
        printf("========================================================\n");
        printf("Use Arrow Keys (Up/Down) to navigate, Enter to select.\n");
        
        int key = get_key();
        if (key == KEY_UP) {
            choice = (choice - 1 + 6) % 6;
        } else if (key == KEY_DOWN) {
            choice = (choice + 1) % 6;
        } else if (key == KEY_ENTER) {
            break;
        }
    }

    if (choice == 5) return;

    // Modify Coordinates
    if (choice == 0 || choice == 4) {
        if (sh->type == SHAPE_LINE) {
            Point p1 = sh->params.line.start;
            Point p2 = sh->params.line.end;
            if (select_point_on_canvas("Select New LINE START point and press Enter:", &p1, NULL, 0, SHAPE_LINE) &&
                select_point_on_canvas("Select New LINE END point and press Enter:", &p2, &p1, 1, SHAPE_LINE)) {
                sh->params.line.start = p1;
                sh->params.line.end = p2;
            }
        } 
        else if (sh->type == SHAPE_RECTANGLE) {
            int rx = sh->params.rect.top_left.x;
            int ry = sh->params.rect.top_left.y;
            Point p1 = {rx, ry};
            Point p2 = {rx + sh->params.rect.width - 1, ry + sh->params.rect.height - 1};
            if (select_point_on_canvas("Select New RECTANGLE START corner point and press Enter:", &p1, NULL, 0, SHAPE_RECTANGLE) &&
                select_point_on_canvas("Select New RECTANGLE OPPOSITE corner point and press Enter:", &p2, &p1, 1, SHAPE_RECTANGLE)) {
                int xmin = (p1.x < p2.x) ? p1.x : p2.x;
                int ymin = (p1.y < p2.y) ? p1.y : p2.y;
                sh->params.rect.top_left.x = xmin;
                sh->params.rect.top_left.y = ymin;
                sh->params.rect.width = abs(p2.x - p1.x) + 1;
                sh->params.rect.height = abs(p2.y - p1.y) + 1;
            }
        } 
        else if (sh->type == SHAPE_CIRCLE) {
            Point center = sh->params.circle.center;
            Point edge = {center.x + sh->params.circle.radius, center.y};
            if (select_point_on_canvas("Select New CIRCLE CENTER point and press Enter:", &center, NULL, 0, SHAPE_CIRCLE) &&
                select_point_on_canvas("Move cursor to set New CIRCLE RADIUS and press Enter:", &edge, &center, 1, SHAPE_CIRCLE)) {
                sh->params.circle.center = center;
                int dx = edge.x - center.x;
                int dy = edge.y - center.y;
                sh->params.circle.radius = (int)(sqrt(dx * dx + dy * dy) + 0.5);
            }
        } 
        else if (sh->type == SHAPE_TRIANGLE) {
            Point p1 = sh->params.triangle.p1;
            Point p2 = sh->params.triangle.p2;
            Point p3 = sh->params.triangle.p3;
            Point preview[2];
            if (select_point_on_canvas("Select New TRIANGLE POINT 1 and press Enter:", &p1, NULL, 0, SHAPE_TRIANGLE)) {
                preview[0] = p1;
                if (select_point_on_canvas("Select New TRIANGLE POINT 2 and press Enter:", &p2, preview, 1, SHAPE_TRIANGLE)) {
                    preview[1] = p2;
                    if (select_point_on_canvas("Select New TRIANGLE POINT 3 and press Enter:", &p3, preview, 2, SHAPE_TRIANGLE)) {
                        sh->params.triangle.p1 = p1;
                        sh->params.triangle.p2 = p2;
                        sh->params.triangle.p3 = p3;
                    }
                }
            }
        }
    }

    // Modify drawing character
    if (choice == 1 || choice == 4) {
        system("cls");
        sh->draw_char = get_draw_char_prompt(sh->draw_char);
    }

    // Modify shape color
    if (choice == 2 || choice == 4) {
        sh->color = prompt_select_color();
    }

    // Modify fill style
    if (choice == 3 || choice == 4) {
        if (sh->type != SHAPE_LINE) {
            sh->is_filled = prompt_select_fill_mode();
        } else {
            printf("\nLines cannot be filled! Press any key to continue...");
            _getch();
        }
    }

    system("cls");
    printf("\nShape ID %d updated successfully! Press any key to continue...", id);
    _getch();
}

void prompt_change_bg() {
    system("cls");
    printf("=================== CHANGE BACKGROUND ===================\n");
    char new_bg;
    if (read_char("Enter new canvas background character (e.g. _, ., space): ", &new_bg)) {
        bg_char = new_bg;
        printf("\nBackground character changed to '%c'. Press any key to continue...", bg_char);
    } else {
        printf("\nBackground changed to space ' '. Press any key to continue...");
        bg_char = ' ';
    }
    _getch();
}

void clear_all_shapes() {
    for (int i = 0; i < MAX_SHAPES; i++) {
        shapes[i].active = 0;
    }
    next_id = 1;
    printf("\nCanvas cleared. All shapes deleted.\n");
}

int main() {
    // Disable buffering on stdout to ensure immediate printing in all environments
    setbuf(stdout, NULL);

    // Initialize standard shapes array to empty
    for (int i = 0; i < MAX_SHAPES; i++) {
        shapes[i].active = 0;
    }

    // Prepopulate with a few shapes to make it interesting from start
    // A filled circle in the center (Red)
    CircleParams c1 = {{40, 10}, 6};
    add_shape(SHAPE_CIRCLE, &c1, '*', COLOR_RED, 1);

    // An outline rectangle around it (Green)
    RectParams r1 = {{25, 3}, 32, 14};
    add_shape(SHAPE_RECTANGLE, &r1, '*', COLOR_GREEN, 0);

    // A diagonal line (Yellow)
    LineParams l1 = {{2, 2}, {18, 16}};
    add_shape(SHAPE_LINE, &l1, '_', COLOR_YELLOW, 0);

    // A filled triangle on the left (Blue)
    TriangleParams t1 = {{4, 15}, {14, 15}, {9, 10}};
    add_shape(SHAPE_TRIANGLE, &t1, '#', COLOR_BLUE, 1);

    int running = 1;
    int selected = 0;
    while (running) {
        render_all_shapes();
        
        system("cls");
        printf("=================== 2D GRAPHICS EDITOR ===================\n");
        display_canvas();
        list_active_shapes();
        
        printf("\nMenu Options (Use Arrow Keys to navigate, Enter to select):\n");
        const char* menu_options[] = {
            "Add Shape",
            "Delete Shape",
            "Modify Shape",
            "Change Background Character",
            "Delete All Shapes (Clear Canvas)",
            "Exit"
        };
        for (int i = 0; i < 6; i++) {
            if (i == selected) {
                printf("\033[1;36m -> %s\033[0m\n", menu_options[i]);
            } else {
                printf("    %s\n", menu_options[i]);
            }
        }
        
        int key = get_key();
        if (key == KEY_UP) {
            selected = (selected - 1 + 6) % 6;
        } else if (key == KEY_DOWN) {
            selected = (selected + 1) % 6;
        } else if (key == KEY_ENTER) {
            switch (selected) {
                case 0:
                    prompt_add_shape();
                    break;
                case 1:
                    prompt_delete_shape();
                    break;
                case 2:
                    prompt_modify_shape();
                    break;
                case 3:
                    prompt_change_bg();
                    break;
                case 4:
                    clear_all_shapes();
                    printf("Press any key to continue...");
                    _getch();
                    break;
                case 5:
                    running = 0;
                    system("cls");
                    printf("Exiting editor. Goodbye!\n");
                    break;
            }
        }
    }

    return 0;
}
