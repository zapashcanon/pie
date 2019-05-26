/*
 * Copyright 2010-2019 Thierry FOURNIER
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License.
 */
#ifndef __PIE_H__
#define __PIE_H__

#include <cairo.h>

struct conf;

typedef size_t (*pie_write_cb)(void *arg, const unsigned char *data, unsigned int len);

struct conf *pie_new(void);
void pie_set_do_back(struct conf *co, int do_back);
void pie_set_back_color(struct conf *co, char *color);
void pie_set_line_width(struct conf *co, double width);
void pie_set_line_color(struct conf *co, char *color);
void pie_set_decal(struct conf *co, double width);
void pie_set_height(struct conf *co, double height);
void pie_set_img_h(struct conf *co, int img_h);
void pie_set_img_w(struct conf *co, int img_w);
void pie_set_do_legend(struct conf *co, int do_legend);
void pie_set_legend_color(struct conf *co, char *color);
void pie_set_legend_size(struct conf *co, double size);
void pie_set_margin(struct conf *co, int size);
void pie_set_ratio(struct conf *co, double size);
void pie_set_title_size(struct conf *co, int size);
void pie_set_title(struct conf *co, char *title);
void pie_set_title_color(struct conf *co, char *color);
int pie_add(struct conf *co, double value, char *color, double extrude, char *name);
void pie_cairo_draw(cairo_t *c, struct conf *co);
void pie_draw(struct conf *co, int mode, const char *file_out);

#endif /* __PIE_H__ */
