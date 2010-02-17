#ifndef __PIE_H__
#define __PIE_H__

#include <cairo.h>

struct color {
	double r;
	double g;
	double b;
	double a;
};

struct coord {
	double x;
	double y;
};

struct portion {
	struct color light;
	struct color dark;
	struct color dark_deg;
	struct color line;
	double line_width;

	struct coord t_cent;
	struct coord t_strt;
	struct coord t_stop;

	struct coord b_cent;
	struct coord b_strt;
	struct coord b_stop;

	double ang_strt;
	double ang_stop;

	double ca_strt;
	double ca_stop;

	char *legend;
	cairo_text_extents_t legend_exts;

	double tmp;
};

struct conf {
	char *out;
	int mode;

	char do_back;
	struct color back;

	double ratio;
	double decal;
	double height;
	double margin;

	double img_w;
	double img_h;

	double pie_w;
	double pie_h;

	double cx; /* centre x */
	double cy; /* centre y */

	double rx; /* rayon_x */
	double ry; /* rayon_y */

	double line_width;
	struct color line_color;

	double title_size;
	const char *title;
	struct color title_color;
	cairo_text_extents_t title_exts;;

	char draw_leg;
	double leg_size;
	struct color leg_color;

	/* data */
	int nb;
	double *part;
	char **color;
	double *extract;
	char **name;
};

void convert_rgba_hex(char *hex, unsigned char alpha, struct color *out);
void pie(cairo_t *c, struct conf *co);

#endif /* __PIE_H__ */
