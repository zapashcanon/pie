#include <stdlib.h>
#include <string.h>

#include <cairo.h>
#include <cairo-ps.h>
#include <cairo-pdf.h>
#include <cairo-svg.h>

#include "pie.h"

cairo_status_t wr(void *closure, const unsigned char *data, unsigned int length)
{
	FILE *out = closure;

	fwrite(data, 1, length, out);

	return CAIRO_STATUS_SUCCESS;
}

void usage() {
	printf(
		" -b <hex>             : Background color (ex: #ffffff)\n"
		" -c <float>           : Pie line width\n"
		" -C <hex>             : Pie line colo\n"
		" -d <float>           : Percent explode.\n"
		"                        values must between 0 and 1\n"
		" -e <float>           : Percent extrusion\n"
		"                        values must between 0 and 1\n"
		" -f <EPS|PNG|PDF|SVG> : Choose output format\n"
		" -h <integer>         : Height in pixel\n"
		" -i <file>            : input file\n"
		" -l <hex>             : Legend color (ex: #ffffff)\n"
		" -L <float>           : Legend size in px\n"
		" -m <float>           : Margin in pixel\n"
		" -o <file>            : Output file name\n"
		"                        '-' is stdout\n"
		" -r <float>           : Ratio between height and with of pie.\n"
		"                        values must between 0 and 1\n"
		" -s <size>            : Title size in px\n"
		" -t <title>           : Graph title\n"
		" -T <hex>             : Title color (ex: #ffffff)\n"
		" -w <float>           : Width in pixel\n"
	);
	exit(0);
}

static inline void get_one(int *nb, int argc)
{
	(*nb)++;
	if ((*nb) >= argc) {
		fprintf(stderr, "argument expected\n");
		exit(1);
	}
}

static inline int add_data(char *in, struct conf *co)
{
	double value;
	double extrude;
	char color[7];
	char *name;
	char *p;
	int i;

	/* first part: part of pie */
	value = strtod(in, &p);
	if (*p != '#') {
		fprintf(stderr, "invalid entry: bad value: \"%s\"\n", in);
		exit(1);
	}

	/* color */
	p++;
	for(i=0; i<6; i++) {
		if ( ! (
		          (*p >= 'a' && *p <= 'f') ||
		          (*p >= 'A' && *p <= 'F') ||
		          (*p >= '0' && *p <= '9')
		       )
		) {
			fprintf(stderr, "invalid entry: bad color: \"%s\"\n", in);
			exit(1);
		}
		color[i] = *p;
		p++;
	}
	if (*p != ':') {
		fprintf(stderr, "invalid entry: bad format: \"%s\"\n", in);
		exit(1);
	}
	color[6] = '\0';

	/* extrude */
	p++;
	if (*p != ':') {
		extrude = strtod(p, &p);
		if (*p != ':') {
			fprintf(stderr, "invalid entry: bad value: \"%s\"\n", in);
			exit(1);
		}
	}

	/* comment */
	p++;
	name = p;

	/* memory */
	i = co->nb;
	co->nb++;
	co->part    = realloc(co->part,    co->nb * sizeof(double));
	co->color   = realloc(co->color,   co->nb * sizeof(char *));
	co->extract = realloc(co->extract, co->nb * sizeof(double));
	co->name    = realloc(co->name,    co->nb * sizeof(char *));

	co->part[i]    = value;
	co->color[i]   = strdup(color);
	co->extract[i] = extrude;
	co->name[i]    = strdup(name);
}

int main(int argc, char *argv[])
{
	cairo_surface_t *s;
	cairo_t *c;
	struct conf co;
	int nb;
	FILE *out;
	FILE *in;

	/* init cof */
	co.out           = NULL;
	co.mode          = 1;
	co.ratio         = -1;
	co.img_w         = -1;
	co.img_h         = -1;
	co.decal         = -1;
	co.height        = -1;
	co.margin        = -1;
	co.do_back       = 0;
	co.title         = NULL;
	co.title_size    = 15;
	co.draw_leg      = 0;
	co.leg_size      = 10;
	co.leg_color.r   = 0x00;
	co.leg_color.g   = 0x00;
	co.leg_color.b   = 0x00;
	co.leg_color.a   = 0xff;
	co.title_color.r = 0x00;
	co.title_color.g = 0x00;
	co.title_color.b = 0x00;
	co.title_color.a = 0xff;
	co.line_width    = 0;
	co.line_color.r  = 0x00;
	co.line_color.g  = 0x00;
	co.line_color.b  = 0x00;
	co.line_color.a  = 0x00;
	co.part          = NULL;
	co.color         = NULL;
	co.extract       = NULL;
	co.name          = NULL;

	if (argc == 1)
		usage();

	/* read command line */
	for (nb=1; nb<argc; nb++) {

		/* on a fini de lire les options */
		if (argv[nb][0] != '-')
			goto parsing_end;

		switch (argv[nb][1]) {

		/* fini */
		case '-':
			nb ++;
			goto parsing_end;
			break;

		/* background color */
		case 'b':
			get_one(&nb, argc);
			co.do_back = 1;
			convert_rgba_hex(argv[nb], 0xff, &co.back);
			break;

		/* line width */
		case 'c':
			get_one(&nb, argc);
			co.line_width = atof(argv[nb]);
			break;

		/* line color */
		case 'C':
			get_one(&nb, argc);
			convert_rgba_hex(argv[nb], 0xff, &co.line_color);
			break;

		/* decal */
		case 'd':
			get_one(&nb, argc);
			co.decal = atof(argv[nb]);
			break;

		/* decal */
		case 'e':
			get_one(&nb, argc);
			co.height = atof(argv[nb]);
			break;

		/* output format */
		case 'f':
			get_one(&nb, argc);
			/**/ if (strcmp(argv[nb], "PNG") == 0)
				co.mode = 1;
			else if (strcmp(argv[nb], "EPS") == 0)
				co.mode = 2;
			else if (strcmp(argv[nb], "PDF") == 0)
				co.mode = 3;
			else if (strcmp(argv[nb], "SVG") == 0)
				co.mode = 4;
			else {
				fprintf(stderr, "unknown format %s\n", argv[nb]);
				exit(1);
			}
			break;

		/* height */
		case 'h':
			get_one(&nb, argc);
			co.img_h = atoi(argv[nb]);
			break;

		/* legend color */
		case 'l':
			get_one(&nb, argc);
			co.draw_leg = 1;
			convert_rgba_hex(argv[nb], 0xff, &co.leg_color);
			break;

		/* legend size */
		case 'L':
			get_one(&nb, argc);
			co.draw_leg = 1;
			co.leg_size = atof(argv[nb]);
			break;

		/* margin */
		case 'm':
			get_one(&nb, argc);
			co.margin = atof(argv[nb]);
			break;

		/* output */
		case 'o':
			get_one(&nb, argc);
			co.out = argv[nb];
			break;

		/* ratio */
		case 'r':
			get_one(&nb, argc);
			co.ratio = atof(argv[nb]);
			if (co.ratio < 0 || co.ratio > 1) {
				fprintf(stderr, "ratio must be >= 0 and <= 1\n");
				exit(1);
			}
			break;

		/* title size */
		case 's':
			get_one(&nb, argc);
			co.title_size = atoi(argv[nb]);
			break;

		/* title */
		case 't':
			get_one(&nb, argc);
			co.title = argv[nb];
			break;

		/* title size */
		case 'T':
			get_one(&nb, argc);
			convert_rgba_hex(argv[nb], 0xff, &co.title_color);
			break;

		/* width */
		case 'w':
			get_one(&nb, argc);
			co.img_w = atoi(argv[nb]);
			break;
		}
	}

parsing_end:

	/* check */
	if (co.out == NULL) {
		fprintf(stderr, "output name is mandatory\n");
		exit(1);
	}

	/* default config */
	if (co.ratio == -1)
		co.ratio = 0.5f;

	if (co.img_w != -1 && co.img_h == -1)
		co.img_h = co.img_w;
	
	else if (co.img_w == -1 && co.img_h != -1)
		co.img_w = co.img_h;
	
	else if (co.img_w == -1 && co.img_h == -1) {
		co.img_w = 100;
		co.img_h = 100;
	}

	if (co.decal == -1)
		co.decal = 0.1;
	
	if (co.height == -1)
		co.height = 0.4;

	if(co.margin == -1)
		co.margin = 10;

	/* open output file */
	if (strcmp(co.out, "-") == 0)
		out = stdout;
	else {
		out = fopen(co.out, "w");
		if (out == NULL) {
			fprintf(stderr, "can't open outout file\n");
			exit(1);
		}
	}

	/* read data */
	co.nb = 0;

	/* load data */
	for (; nb<argc; nb++)
		add_data(argv[nb], &co);

	/* create image */

	switch (co.mode) {

	/* PNG */
	case 1:
		s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, co.img_w, co.img_h);
		break;

	/* EPS */
	case 2:
		s = cairo_ps_surface_create_for_stream(wr, out, co.img_w, co.img_h);
		cairo_ps_surface_set_eps(s, 1);
		break;

	/* SVG */
	case 3:
		s = cairo_svg_surface_create_for_stream(wr, out, co.img_w, co.img_h);
		break;

	/* PDF */
	case 4:
		s = cairo_pdf_surface_create_for_stream(wr, out, co.img_w, co.img_h);
		break;
	}

	/* create cairo */
	c = cairo_create(s);

	/* trace path */
	pie(c, &co);
	cairo_close_path(c);
	cairo_show_page(c);

	/* write image */
	if (co.mode == 1)
		cairo_surface_write_to_png_stream(s, wr, out);

	/* general vectoriel */
	else {
		cairo_surface_flush(s);
		cairo_surface_finish(s);
		cairo_surface_destroy(s);
		cairo_destroy(c);
	}

	/* end */
	fflush(out);
	fclose(out);
}