/*
 * 2D Graphics Editor
 * Uses * and _ characters to draw shapes on a 2D character canvas.
 * Supports: Circle, Rectangle, Line, Triangle
 * Operations: Add, Delete, Modify objects
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ─── Canvas dimensions ─────────────────────────────────── */
#define ROWS  30
#define COLS  80
#define EMPTY ' '
#define FILL  '*'
#define BORDER '_'

/* ─── Shape types ────────────────────────────────────────── */
#define SHAPE_CIRCLE    1
#define SHAPE_RECTANGLE 2
#define SHAPE_LINE      3
#define SHAPE_TRIANGLE  4
#define MAX_OBJECTS    50

/* ─── Data structures ────────────────────────────────────── */
typedef struct {
    int type;          /* SHAPE_* constant          */
    int x1, y1;        /* primary point / centre    */
    int x2, y2;        /* secondary point / radius  */
    int x3, y3;        /* third point (triangle)    */
    int radius;        /* circle radius             */
    char fill_char;    /* '*' or '_'                */
    int active;        /* 1 = present, 0 = deleted  */
} Shape;

/* ─── Globals ────────────────────────────────────────────── */
char canvas[ROWS][COLS];
Shape objects[MAX_OBJECTS];
int  object_count = 0;

/* ══════════════════════════════════════════════════════════
 *  Canvas helpers
 * ══════════════════════════════════════════════════════════ */
void init_canvas(void) {
    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            canvas[r][c] = EMPTY;
}

/* Safe plot – ignores out-of-bounds coordinates */
void plot(int r, int c, char ch) {
    if (r >= 0 && r < ROWS && c >= 0 && c < COLS)
        canvas[r][c] = ch;
}

void display_canvas(void) {
    /* Top border */
    printf("+");
    for (int c = 0; c < COLS; c++) printf("-");
    printf("+\n");

    for (int r = 0; r < ROWS; r++) {
        printf("|");
        for (int c = 0; c < COLS; c++)
            putchar(canvas[r][c]);
        printf("|\n");
    }

    /* Bottom border */
    printf("+");
    for (int c = 0; c < COLS; c++) printf("-");
    printf("+\n");
}

/* ══════════════════════════════════════════════════════════
 *  Drawing primitives
 * ══════════════════════════════════════════════════════════ */

/* Bresenham line */
void draw_line(int r1, int c1, int r2, int c2, char ch) {
    int dr = abs(r2 - r1), dc = abs(c2 - c1);
    int sr = (r1 < r2) ? 1 : -1;
    int sc = (c1 < c2) ? 1 : -1;
    int err = dr - dc;

    while (1) {
        plot(r1, c1, ch);
        if (r1 == r2 && c1 == c2) break;
        int e2 = 2 * err;
        if (e2 > -dc) { err -= dc; r1 += sr; }
        if (e2 <  dr) { err += dr; c1 += sc; }
    }
}

/* Midpoint circle (outline only) */
void draw_circle(int cr, int cc, int radius, char ch) {
    int x = 0, y = radius;
    int d = 1 - radius;

    while (x <= y) {
        plot(cr + y, cc + x, ch);
        plot(cr - y, cc + x, ch);
        plot(cr + y, cc - x, ch);
        plot(cr - y, cc - x, ch);
        plot(cr + x, cc + y, ch);
        plot(cr - x, cc + y, ch);
        plot(cr + x, cc - y, ch);
        plot(cr - x, cc - y, ch);

        if (d < 0)
            d += 2 * x + 3;
        else {
            d += 2 * (x - y) + 5;
            y--;
        }
        x++;
    }
}

/* Axis-aligned rectangle (outline) */
void draw_rectangle(int r1, int c1, int r2, int c2, char ch) {
    for (int c = c1; c <= c2; c++) { plot(r1, c, ch); plot(r2, c, ch); }
    for (int r = r1; r <= r2; r++) { plot(r, c1, ch); plot(r, c2, ch); }
}

/* Triangle via three line segments */
void draw_triangle(int r1, int c1, int r2, int c2, int r3, int c3, char ch) {
    draw_line(r1, c1, r2, c2, ch);
    draw_line(r2, c2, r3, c3, ch);
    draw_line(r3, c3, r1, c1, ch);
}

/* ══════════════════════════════════════════════════════════
 *  Redraw entire canvas from object list
 * ══════════════════════════════════════════════════════════ */
void redraw_all(void) {
    init_canvas();
    for (int i = 0; i < object_count; i++) {
        if (!objects[i].active) continue;
        Shape *s = &objects[i];
        switch (s->type) {
            case SHAPE_CIRCLE:
                draw_circle(s->x1, s->y1, s->radius, s->fill_char);
                break;
            case SHAPE_RECTANGLE:
                draw_rectangle(s->x1, s->y1, s->x2, s->y2, s->fill_char);
                break;
            case SHAPE_LINE:
                draw_line(s->x1, s->y1, s->x2, s->y2, s->fill_char);
                break;
            case SHAPE_TRIANGLE:
                draw_triangle(s->x1, s->y1, s->x2, s->y2,
                              s->x3, s->y3, s->fill_char);
                break;
        }
    }
}

/* ══════════════════════════════════════════════════════════
 *  Object management
 * ══════════════════════════════════════════════════════════ */
int add_object(Shape s) {
    if (object_count >= MAX_OBJECTS) {
        printf("Canvas full – cannot add more objects.\n");
        return -1;
    }
    s.active = 1;
    objects[object_count] = s;
    return object_count++;
}

void delete_object(int id) {
    if (id < 0 || id >= object_count || !objects[id].active) {
        printf("Invalid object ID.\n");
        return;
    }
    objects[id].active = 0;
    printf("Object %d deleted.\n", id);
    redraw_all();
}

/* Generic modify: replace the shape with a new one provided by caller */
void modify_object(int id, Shape new_shape) {
    if (id < 0 || id >= object_count || !objects[id].active) {
        printf("Invalid object ID.\n");
        return;
    }
    new_shape.active = 1;
    new_shape.type   = objects[id].type;   /* keep original type */
    objects[id]      = new_shape;
    redraw_all();
    printf("Object %d modified.\n", id);
}

/* ══════════════════════════════════════════════════════════
 *  Input helpers
 * ══════════════════════════════════════════════════════════ */
void flush_input(void) { int c; while ((c = getchar()) != '\n' && c != EOF); }

char choose_char(void) {
    int ch;
    printf("Draw with * or _ ? Enter character: ");
    ch = getchar(); flush_input();
    if (ch == '_') return '_';
    return '*';
}

void list_objects(void) {
    int found = 0;
    printf("\n%-4s %-12s Details\n", "ID", "Type");
    printf("---------------------------------------------\n");
    for (int i = 0; i < object_count; i++) {
        if (!objects[i].active) continue;
        found = 1;
        const char *name;
        switch (objects[i].type) {
            case SHAPE_CIRCLE:    name = "Circle";    break;
            case SHAPE_RECTANGLE: name = "Rectangle"; break;
            case SHAPE_LINE:      name = "Line";      break;
            case SHAPE_TRIANGLE:  name = "Triangle";  break;
            default:              name = "Unknown";
        }
        printf("%-4d %-12s", i, name);
        switch (objects[i].type) {
            case SHAPE_CIRCLE:
                printf("centre(%d,%d) r=%d char=%c",
                       objects[i].x1, objects[i].y1,
                       objects[i].radius, objects[i].fill_char);
                break;
            case SHAPE_RECTANGLE:
                printf("(%d,%d)-(%d,%d) char=%c",
                       objects[i].x1, objects[i].y1,
                       objects[i].x2, objects[i].y2,
                       objects[i].fill_char);
                break;
            case SHAPE_LINE:
                printf("(%d,%d)->(%d,%d) char=%c",
                       objects[i].x1, objects[i].y1,
                       objects[i].x2, objects[i].y2,
                       objects[i].fill_char);
                break;
            case SHAPE_TRIANGLE:
                printf("(%d,%d),(%d,%d),(%d,%d) char=%c",
                       objects[i].x1, objects[i].y1,
                       objects[i].x2, objects[i].y2,
                       objects[i].x3, objects[i].y3,
                       objects[i].fill_char);
                break;
        }
        putchar('\n');
    }
    if (!found) printf("(no active objects)\n");
    printf("---------------------------------------------\n");
}

/* ══════════════════════════════════════════════════════════
 *  Menu: Add sub-menu
 * ══════════════════════════════════════════════════════════ */
void menu_add(void) {
    printf("\n--- Add Shape ---\n");
    printf("1. Circle\n2. Rectangle\n3. Line\n4. Triangle\n0. Back\n> ");
    int choice; scanf("%d", &choice); flush_input();

    Shape s; memset(&s, 0, sizeof(s));
    s.fill_char = choose_char();

    switch (choice) {
        case 1:
            s.type = SHAPE_CIRCLE;
            printf("Centre row col radius: ");
            scanf("%d %d %d", &s.x1, &s.y1, &s.radius); flush_input();
            draw_circle(s.x1, s.y1, s.radius, s.fill_char);
            break;
        case 2:
            s.type = SHAPE_RECTANGLE;
            printf("Top-left row col, bottom-right row col: ");
            scanf("%d %d %d %d", &s.x1, &s.y1, &s.x2, &s.y2); flush_input();
            draw_rectangle(s.x1, s.y1, s.x2, s.y2, s.fill_char);
            break;
        case 3:
            s.type = SHAPE_LINE;
            printf("Start row col, end row col: ");
            scanf("%d %d %d %d", &s.x1, &s.y1, &s.x2, &s.y2); flush_input();
            draw_line(s.x1, s.y1, s.x2, s.y2, s.fill_char);
            break;
        case 4:
            s.type = SHAPE_TRIANGLE;
            printf("Vertex1 row col: "); scanf("%d %d", &s.x1, &s.y1); flush_input();
            printf("Vertex2 row col: "); scanf("%d %d", &s.x2, &s.y2); flush_input();
            printf("Vertex3 row col: "); scanf("%d %d", &s.x3, &s.y3); flush_input();
            draw_triangle(s.x1, s.y1, s.x2, s.y2, s.x3, s.y3, s.fill_char);
            break;
        default:
            return;
    }
    int id = add_object(s);
    if (id >= 0) printf("Shape added with ID %d.\n", id);
}

/* ══════════════════════════════════════════════════════════
 *  Menu: Delete sub-menu
 * ══════════════════════════════════════════════════════════ */
void menu_delete(void) {
    list_objects();
    printf("Enter object ID to delete (-1 to cancel): ");
    int id; scanf("%d", &id); flush_input();
    if (id >= 0) delete_object(id);
}

/* ══════════════════════════════════════════════════════════
 *  Menu: Modify sub-menu
 * ══════════════════════════════════════════════════════════ */
void menu_modify(void) {
    list_objects();
    printf("Enter object ID to modify (-1 to cancel): ");
    int id; scanf("%d", &id); flush_input();
    if (id < 0 || id >= object_count || !objects[id].active) {
        printf("Invalid ID.\n"); return;
    }

    Shape s = objects[id];   /* copy current values as defaults */
    s.fill_char = choose_char();

    switch (s.type) {
        case SHAPE_CIRCLE:
            printf("New centre row col radius (current %d %d %d): ",
                   s.x1, s.y1, s.radius);
            scanf("%d %d %d", &s.x1, &s.y1, &s.radius); flush_input();
            break;
        case SHAPE_RECTANGLE:
            printf("New top-left row col, bottom-right row col (current %d %d %d %d): ",
                   s.x1, s.y1, s.x2, s.y2);
            scanf("%d %d %d %d", &s.x1, &s.y1, &s.x2, &s.y2); flush_input();
            break;
        case SHAPE_LINE:
            printf("New start row col, end row col (current %d %d %d %d): ",
                   s.x1, s.y1, s.x2, s.y2);
            scanf("%d %d %d %d", &s.x1, &s.y1, &s.x2, &s.y2); flush_input();
            break;
        case SHAPE_TRIANGLE:
            printf("New vertex1 (current %d %d): ", s.x1, s.y1);
            scanf("%d %d", &s.x1, &s.y1); flush_input();
            printf("New vertex2 (current %d %d): ", s.x2, s.y2);
            scanf("%d %d", &s.x2, &s.y2); flush_input();
            printf("New vertex3 (current %d %d): ", s.x3, s.y3);
            scanf("%d %d", &s.x3, &s.y3); flush_input();
            break;
    }
    modify_object(id, s);
}

/* ══════════════════════════════════════════════════════════
 *  Demo: pre-populate a sample scene
 * ══════════════════════════════════════════════════════════ */
void load_demo(void) {
    Shape s; memset(&s, 0, sizeof(s));

    /* Rectangle border */
    s.type = SHAPE_RECTANGLE; s.x1=1; s.y1=1; s.x2=28; s.y2=78; s.fill_char='_';
    add_object(s);

    /* Circle */
    s.type = SHAPE_CIRCLE; s.x1=10; s.y1=20; s.radius=6; s.fill_char='*';
    add_object(s);

    /* Line */
    s.type = SHAPE_LINE; s.x1=2; s.y1=2; s.x2=27; s.y2=77; s.fill_char='*';
    add_object(s);

    /* Triangle */
    s.type = SHAPE_TRIANGLE;
    s.x1=5; s.y1=50; s.x2=15; s.y2=40; s.x3=15; s.y3=60; s.fill_char='*';
    add_object(s);

    redraw_all();
    printf("Demo scene loaded (4 objects).\n");
}

/* ══════════════════════════════════════════════════════════
 *  Main menu loop
 * ══════════════════════════════════════════════════════════ */
int main(void) {
    init_canvas();
    printf("╔══════════════════════════════════════╗\n");
    printf("║    2D Graphics Editor (C / ASCII)    ║\n");
    printf("╚══════════════════════════════════════╝\n");
    printf("Canvas: %d rows × %d cols  |  chars: * and _\n\n", ROWS, COLS);

    int choice;
    do {
        printf("\n══ MAIN MENU ══════════════════════════\n");
        printf(" 1. Display canvas\n");
        printf(" 2. Add shape\n");
        printf(" 3. Delete shape\n");
        printf(" 4. Modify shape\n");
        printf(" 5. List objects\n");
        printf(" 6. Clear canvas\n");
        printf(" 7. Load demo scene\n");
        printf(" 0. Exit\n");
        printf("══════════════════════════════════════\n> ");
        scanf("%d", &choice); flush_input();

        switch (choice) {
            case 1: redraw_all(); display_canvas(); break;
            case 2: menu_add();   redraw_all(); display_canvas(); break;
            case 3: menu_delete(); display_canvas(); break;
            case 4: menu_modify(); display_canvas(); break;
            case 5: list_objects(); break;
            case 6:
                init_canvas();
                object_count = 0;
                printf("Canvas cleared.\n");
                break;
            case 7: load_demo(); display_canvas(); break;
            case 0: printf("Goodbye!\n"); break;
            default: printf("Invalid option.\n");
        }
    } while (choice != 0);

    return 0;
}