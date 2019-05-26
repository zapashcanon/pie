/*
 * Copyright 2010-2019 Thierry FOURNIER
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License.
 */
#ifndef __ELLIPSE_H__
#define __ELLIPSE_H__

#include <cairo.h>

void cairo_ellipse(cairo_t *c, double x, double y,
                               double rayon_x, double rayon_y,
                               double angle_start, double angle_stop);
void cairo_ellipse_negative(cairo_t *c, double x, double y,
                                        double rayon_x, double rayon_y,
                                        double angle_start, double angle_stop);

#endif /* __ELLIPSE_H__ */
