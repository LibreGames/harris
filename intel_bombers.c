/*
	harris - a strategy game
	Copyright (C) 2012-2014 Edward Cree

	licensed under GPLv3+ - see top of harris.c for details
	
	intel_bombers: bombers' intel screen
*/

#include "intel_bombers.h"
#include <stdbool.h>
#include "ui.h"
#include "globals.h"
#include "bits.h"
#include "date.h"

atg_box *intel_bombers_box;
atg_element *IB_full, *IB_bombers_tab, *IB_fighters_tab, *IB_targets_tab, *IB_cont;
atg_element **IB_types, **IB_namebox, *IB_side_image, *IB_text_box, *IB_stat_box;
unsigned int IB_i;
SDL_Surface *IB_blank;

void update_intel_bombers(const game *state);

enum b_stat_i
{
	/* Numbers with units */
	STAT_COST,
	STAT_SPEED,
	STAT_CEILING,
	STAT_LOAD,
	STAT_RANGE,
	/* Woolly gameplay numbers */
	STAT_SV,
	STAT_DE,
	STAT_FA,
	STAT_AC,

	NUM_STATS
};

struct b_stat_row
{
	const char *name;
	char value_buf[6];
	const char *unit;
	bool unit_first;
	unsigned int bar_min, bar_max;
	bool bar_rev; // reverse colours
	size_t v_off;
	int v_shift, v_scale; // shift is applied first
}
b_stat_rows[NUM_STATS]=
{
	[STAT_COST]   ={.name="Cost",           .unit="£",   .unit_first=true,  .bar_min=0,     .bar_max=50000, .v_off=offsetof(bombertype, cost ), .v_shift=0,   .v_scale=1,   .bar_rev=true },
	[STAT_SPEED]  ={.name="Speed",          .unit="mph", .unit_first=false, .bar_min=120,   .bar_max=275,   .v_off=offsetof(bombertype, speed), .v_shift=0,   .v_scale=1,   .bar_rev=false},
	[STAT_CEILING]={.name="Ceiling",        .unit="ft",  .unit_first=false, .bar_min=16000, .bar_max=26000, .v_off=offsetof(bombertype, alt  ), .v_shift=0,   .v_scale=100, .bar_rev=false},
	[STAT_LOAD]   ={.name="Max. Bombload",  .unit="lb",  .unit_first=false, .bar_min=0,     .bar_max=20000, .v_off=offsetof(bombertype, cap  ), .v_shift=0,   .v_scale=1,   .bar_rev=false},
	[STAT_SV]     ={.name="Serviceability", .unit="%",   .unit_first=false, .bar_min=0,     .bar_max=100,   .v_off=offsetof(bombertype, svp  ), .v_shift=0,   .v_scale=1,   .bar_rev=false},
	[STAT_DE]     ={.name="Survivability",  .unit=" ",   .unit_first=false, .bar_min=0,     .bar_max=20,    .v_off=offsetof(bombertype, defn ), .v_shift=-20, .v_scale=-1,  .bar_rev=false},
	[STAT_FA]     ={.name="Reliability",    .unit=" ",   .unit_first=false, .bar_min=0,     .bar_max=20,    .v_off=offsetof(bombertype, fail ), .v_shift=-20, .v_scale=-1,  .bar_rev=false},
	[STAT_AC]     ={.name="Accuracy",       .unit=" ",   .unit_first=false, .bar_min=20,    .bar_max=80,    .v_off=offsetof(bombertype, accu ), .v_shift=0,   .v_scale=1,   .bar_rev=false},
	[STAT_RANGE]  ={.name="Max. Range",     .unit="mi",  .unit_first=false, .bar_min=450,   .bar_max=1200,  .v_off=offsetof(bombertype, range), .v_shift=0,   .v_scale=3,   .bar_rev=false},
};

int intel_bombers_create(void)
{
	if(!(intel_bombers_box=atg_create_box(ATG_BOX_PACK_VERTICAL, GAME_BG_COLOUR)))
	{
		fprintf(stderr, "atg_create_box failed\n");
		return(1);
	}
	atg_element *title=atg_create_element_label("HARRIS: Intel & Information", 12, (atg_colour){239, 239, 239, ATG_ALPHA_OPAQUE});
	if(!title)
	{
		fprintf(stderr, "atg_create_element_label failed\n");
		return(1);
	}
	if(atg_pack_element(intel_bombers_box, title))
	{
		perror("atg_pack_element");
		return(1);
	}
	atg_element *tab_box=atg_create_element_box(ATG_BOX_PACK_HORIZONTAL, GAME_BG_COLOUR);
	if(!tab_box)
	{
		fprintf(stderr, "atg_create_element_box failed\n");
		return(1);
	}
	if(atg_pack_element(intel_bombers_box, tab_box))
	{
		perror("atg_pack_element");
		return(1);
	}
	atg_box *tb=tab_box->elem.box;
	if(!tb)
	{
		fprintf(stderr, "tab_box->elem.box==NULL\n");
		return(1);
	}
	if(!(IB_bombers_tab=atg_create_element_button("Bombers", (atg_colour){47, 47, 47, ATG_ALPHA_OPAQUE}, (atg_colour){239, 239, 239, ATG_ALPHA_OPAQUE})))
	{
		fprintf(stderr, "atg_create_element_button failed\n");
		return(1);
	}
	if(atg_pack_element(tb, IB_bombers_tab))
	{
		perror("atg_pack_element");
		return(1);
	}
	if(!(IB_fighters_tab=atg_create_element_button("Fighters", (atg_colour){239, 239, 239, ATG_ALPHA_OPAQUE}, (atg_colour){47, 47, 47, ATG_ALPHA_OPAQUE})))
	{
		fprintf(stderr, "atg_create_element_button failed\n");
		return(1);
	}
	if(atg_pack_element(tb, IB_fighters_tab))
	{
		perror("atg_pack_element");
		return(1);
	}
	if(!(IB_targets_tab=atg_create_element_button("Targets", (atg_colour){239, 239, 239, ATG_ALPHA_OPAQUE}, (atg_colour){47, 47, 47, ATG_ALPHA_OPAQUE})))
	{
		fprintf(stderr, "atg_create_element_button failed\n");
		return(1);
	}
	if(atg_pack_element(tb, IB_targets_tab))
	{
		perror("atg_pack_element");
		return(1);
	}
	if(!(IB_cont=atg_create_element_button("Continue", (atg_colour){239, 239, 239, ATG_ALPHA_OPAQUE}, (atg_colour){63, 63, 63, ATG_ALPHA_OPAQUE})))
	{
		fprintf(stderr, "atg_create_element_button failed\n");
		return(1);
	}
	if(atg_pack_element(tb, IB_cont))
	{
		perror("atg_pack_element");
		return(1);
	}
	IB_full=atg_create_element_image(fullbtn);
	if(!IB_full)
	{
		fprintf(stderr, "atg_create_element_image failed\n");
		return(1);
	}
	IB_full->clickable=true;
	if(atg_pack_element(tb, IB_full))
	{
		perror("atg_pack_element");
		return(1);
	}
	atg_element *main_box=atg_create_element_box(ATG_BOX_PACK_HORIZONTAL, GAME_BG_COLOUR);
	if(!main_box)
	{
		fprintf(stderr, "atg_create_element_box failed\n");
		return(1);
	}
	main_box->w=default_w;
	main_box->h=default_h-18-14;
	if(atg_pack_element(intel_bombers_box, main_box))
	{
		perror("atg_pack_element");
		return(1);
	}
	atg_box *mb=main_box->elem.box;
	if(!mb)
	{
		fprintf(stderr, "main_box->elem.box==NULL\n");
		return(1);
	}
	atg_element *left_box=atg_create_element_box(ATG_BOX_PACK_VERTICAL, GAME_BG_COLOUR);
	if(!left_box)
	{
		fprintf(stderr, "atg_create_element_box failed\n");
		return(1);
	}
	if(atg_pack_element(mb, left_box))
	{
		perror("atg_pack_element");
		return(1);
	}
	atg_box *lb=left_box->elem.box;
	if(!lb)
	{
		fprintf(stderr, "left_box->elem.box==NULL\n");
		return(1);
	}
	atg_element *btlbl=atg_create_element_label("Bomber Types", 12, (atg_colour){223, 223, 223, ATG_ALPHA_OPAQUE});
	if(!btlbl)
	{
		fprintf(stderr, "atg_create_element_label failed\n");
		return(1);
	}
	if(atg_pack_element(lb, btlbl))
	{
		perror("atg_pack_element");
		return(1);
	}
	if(!(IB_types=calloc(ntypes, sizeof(atg_element *))))
	{
		perror("calloc");
		return(1);
	}
	if(!(IB_namebox=calloc(ntypes, sizeof(atg_element *))))
	{
		perror("calloc");
		return(1);
	}
	for(unsigned int i=0;i<ntypes;i++)
	{
		if(!(IB_types[i]=atg_create_element_box(ATG_BOX_PACK_HORIZONTAL, GAME_BG_COLOUR)))
		{
			fprintf(stderr, "atg_create_element_box failed\n");
			return(1);
		}
		IB_types[i]->w=160;
		IB_types[i]->clickable=true;
		if(atg_pack_element(lb, IB_types[i]))
		{
			perror("atg_pack_element");
			return(1);
		}
		atg_box *bb=IB_types[i]->elem.box;
		if(!bb)
		{
			fprintf(stderr, "IB_types[%u]->elem.box==NULL\n", i);
			return(1);
		}
		SDL_Surface *pic=SDL_CreateRGBSurface(SDL_HWSURFACE, 36, 40, types[i].picture->format->BitsPerPixel, types[i].picture->format->Rmask, types[i].picture->format->Gmask, types[i].picture->format->Bmask, types[i].picture->format->Amask);
		if(!pic)
		{
			fprintf(stderr, "pic=SDL_CreateRGBSurface: %s\n", SDL_GetError());
			return(1);
		}
		SDL_FillRect(pic, &(SDL_Rect){0, 0, pic->w, pic->h}, SDL_MapRGB(pic->format, 0, 0, 0));
		SDL_BlitSurface(types[i].picture, NULL, pic, &(SDL_Rect){(36-types[i].picture->w)>>1, (40-types[i].picture->h)>>1, 0, 0});
		atg_element *bpic=atg_create_element_image(pic);
		SDL_FreeSurface(pic);
		if(!bpic)
		{
			fprintf(stderr, "atg_create_element_image failed\n");
			return(1);
		}
		bpic->w=38;
		bpic->h=42;
		bpic->cache=true;
		if(atg_pack_element(bb, bpic))
		{
			perror("atg_pack_element");
			return(1);
		}
		IB_namebox[i]=atg_create_element_box(ATG_BOX_PACK_VERTICAL, (atg_colour){31, 31, 31, ATG_ALPHA_OPAQUE});
		if(!IB_namebox[i])
		{
			fprintf(stderr, "atg_create_element_box failed\n");
			return(1);
		}
		IB_namebox[i]->w=122;
		IB_namebox[i]->h=42;
		if(atg_pack_element(bb, IB_namebox[i]))
		{
			perror("atg_pack_element");
			return(1);
		}
		atg_box *nb=IB_namebox[i]->elem.box;
		if(!nb)
		{
			fprintf(stderr, "IB_namebox[%u]->elem.box==NULL\n", i);
			return(1);
		}
		if(types[i].manu&&types[i].name)
		{
			atg_element *manu=atg_create_element_label(types[i].manu, 10, (atg_colour){175, 199, 255, ATG_ALPHA_OPAQUE});
			if(!manu)
			{
				fprintf(stderr, "atg_create_element_label failed\n");
				return(1);
			}
			if(atg_pack_element(nb, manu))
			{
				perror("atg_pack_element");
				return(1);
			}
			atg_element *name=atg_create_element_label(types[i].name, 14, (atg_colour){175, 199, 255, ATG_ALPHA_OPAQUE});
			if(!name)
			{
				fprintf(stderr, "atg_create_element_label failed\n");
				return(1);
			}
			if(atg_pack_element(nb, name))
			{
				perror("atg_pack_element");
				return(1);
			}
		}
		else
		{
			fprintf(stderr, "Missing manu or name in type %u\n", i);
			return(1);
		}
	}
	atg_element *right_box=atg_create_element_box(ATG_BOX_PACK_VERTICAL, GAME_BG_COLOUR);
	if(!right_box)
	{
		fprintf(stderr, "atg_create_element_box failed\n");
		return(1);
	}
	if(atg_pack_element(mb, right_box))
	{
		perror("atg_pack_element");
		return(1);
	}
	atg_box *rb=right_box->elem.box;
	if(!rb)
	{
		fprintf(stderr, "right_box->elem.box==NULL\n");
		return(1);
	}
	if(!(IB_blank=SDL_CreateRGBSurface(SDL_HWSURFACE, 414, 150, 24, 0xff0000, 0xff00, 0xff, 0)))
	{
		fprintf(stderr, "IB_blank: SDL_CreateRGBSurface: %s\n", SDL_GetError());
		return(1);
	}
	SDL_FillRect(IB_blank, &(SDL_Rect){0, 0, IB_blank->w, IB_blank->h}, SDL_MapRGB(IB_blank->format, 0, 0, 23));
	IB_side_image=atg_create_element_image(IB_blank);
	if(!IB_side_image)
	{
		fprintf(stderr, "atg_create_element_image failed\n");
		return(1);
	}
	IB_side_image->w=414;
	IB_side_image->h=150;
	if(atg_pack_element(rb, IB_side_image))
	{
		perror("atg_pack_element");
		return(1);
	}
	if(!(IB_stat_box=atg_create_element_box(ATG_BOX_PACK_VERTICAL, (atg_colour){223, 223, 223, ATG_ALPHA_OPAQUE})))
	{
		fprintf(stderr, "atg_create_element_box failed\n");
		return(1);
	}
	IB_stat_box->w=414;
	if(atg_pack_element(rb, IB_stat_box))
	{
		perror("atg_pack_element");
		return(1);
	}
	atg_element *text_guard=atg_create_element_box(ATG_BOX_PACK_HORIZONTAL, (atg_colour){239, 239, 239, ATG_ALPHA_OPAQUE});
	if(!text_guard)
	{
		fprintf(stderr, "atg_create_element_box failed\n");
		return(1);
	}
	text_guard->w=414;
	if(atg_pack_element(rb, text_guard))
	{
		perror("atg_pack_element");
		return(1);
	}
	atg_box *tgb=text_guard->elem.box;
	if(!tgb)
	{
		fprintf(stderr, "text_guard->elem.box==NULL\n");
		return(1);
	}
	atg_element *text_shim=atg_create_element_box(ATG_BOX_PACK_VERTICAL, (atg_colour){239, 239, 239, ATG_ALPHA_OPAQUE});
	if(!text_shim)
	{
		fprintf(stderr, "atg_create_element_box failed\n");
		return(1);
	}
	text_shim->w=2;
	text_shim->h=8;
	if(atg_pack_element(tgb, text_shim))
	{
		perror("atg_pack_element");
		return(1);
	}
	if(!(IB_text_box=atg_create_element_box(ATG_BOX_PACK_VERTICAL, (atg_colour){239, 239, 239, ATG_ALPHA_OPAQUE})))
	{
		fprintf(stderr, "atg_create_element_box failed\n");
		return(1);
	}
	IB_text_box->w=410;
	if(atg_pack_element(tgb, IB_text_box))
	{
		perror("atg_pack_element");
		return(1);
	}
	return(0);
}

screen_id intel_bombers_screen(atg_canvas *canvas, game *state)
{
	atg_event e;
	atg_resize_canvas(canvas, mainsizex, mainsizey);
	while(1)
	{
		update_intel_bombers(state);
		atg_flip(canvas);
		while(atg_poll_event(&e, canvas))
		{
			switch(e.type)
			{
				case ATG_EV_RAW:;
					SDL_Event s=e.event.raw;
					switch(s.type)
					{
						case SDL_QUIT:
							mainsizex=canvas->surface->w;
							mainsizey=canvas->surface->h;
							return(intel_caller);
					}
				break;
				case ATG_EV_CLICK:;
					atg_ev_click c=e.event.click;
					if(c.e==IB_full)
					{
						fullscreen=!fullscreen;
						atg_setopts_canvas(canvas, fullscreen?SDL_FULLSCREEN:SDL_RESIZABLE);
					}
					else if(c.e)
					{
						unsigned int i;
						for(i=0;i<ntypes;i++)
						{
							if(c.e==IB_types[i])
								break;
						}
						if(i<ntypes)
						{
							IB_i=i;
						}
						else
						{
							fprintf(stderr, "Clicked on unknown clickable!\n");
						}
					}
				break;
				case ATG_EV_TRIGGER:;
					atg_ev_trigger trigger=e.event.trigger;
					if(trigger.e==IB_bombers_tab)
					{
						// do nothing
					}
					else if(trigger.e==IB_fighters_tab)
					{
						mainsizex=canvas->surface->w;
						mainsizey=canvas->surface->h;
						return(SCRN_INTELFTR);
					}
					else if(trigger.e==IB_targets_tab)
					{
						mainsizex=canvas->surface->w;
						mainsizey=canvas->surface->h;
						return(SCRN_INTELTRG);
					}
					else if(trigger.e==IB_cont)
					{
						mainsizex=canvas->surface->w;
						mainsizey=canvas->surface->h;
						return(intel_caller);
					}
					else if(!trigger.e)
					{
						// internal error
					}
					else
						fprintf(stderr, "Clicked on unknown button!\n");
				break;
				default:
				break;
			}
		}
		SDL_Delay(50);
	}
}

void intel_bombers_free(void)
{
	atg_free_box_box(intel_bombers_box);
}

void update_intel_bombers(const game *state)
{
	for(unsigned int i=0;i<ntypes;i++)
	{
		IB_types[i]->hidden=datebefore(state->now, types[i].entry);
		atg_box *nb=IB_namebox[i]->elem.box;
		if(nb)
		{
			if(i==IB_i)
				nb->bgcolour=(atg_colour){47, 47, 63, ATG_ALPHA_OPAQUE};
			else if(!datebefore(state->now, types[i].exit))
				nb->bgcolour=(atg_colour){15, 15, 63, ATG_ALPHA_OPAQUE};
			else
				nb->bgcolour=(atg_colour){31, 31, 31, ATG_ALPHA_OPAQUE};
		}
	}
	atg_image *si=IB_side_image->elem.image;
	if(si)
	{
		if(types[IB_i].side_image)
			si->data=types[IB_i].side_image;
		else
			si->data=IB_blank;
	}
	atg_box *ntb=atg_create_box(ATG_BOX_PACK_VERTICAL, (atg_colour){239, 239, 239, ATG_ALPHA_OPAQUE});
	if(ntb)
	{
		atg_free_box_box(IB_text_box->elem.box);
		IB_text_box->elem.box=ntb;
		const char *bodytext=types[IB_i].text;
		size_t x=0;
		while(bodytext && bodytext[x])
		{
			size_t l=strcspn(bodytext+x, "\n");
			if(!l) break;
			char *t=strndup(bodytext+x, l);
			atg_element *r=atg_create_element_label(t, 10, (atg_colour){0, 0, 0, ATG_ALPHA_OPAQUE});
			free(t);
			if(!r)
			{
				fprintf(stderr, "atg_create_element_label failed\n");
				break;
			}
			if(atg_pack_element(ntb, r))
			{
				perror("atg_pack_element");
				atg_free_element(r);
				break;
			}
			x+=l;
			if(bodytext[x]=='\n') x++;
		}
	}
	atg_box *nsb=atg_create_box(ATG_BOX_PACK_VERTICAL, (atg_colour){223, 223, 223, ATG_ALPHA_OPAQUE});
	if(nsb)
	{
		atg_free_box_box(IB_stat_box->elem.box);
		IB_stat_box->elem.box=nsb;
		for(unsigned int i=0;i<NUM_STATS;i++)
		{
			atg_element *row=atg_create_element_box(ATG_BOX_PACK_HORIZONTAL, (atg_colour){223, 223, 223, ATG_ALPHA_OPAQUE});
			if(!row)
			{
				fprintf(stderr, "atg_create_element_box failed\n");
				break;
			}
			if(atg_pack_element(nsb, row))
			{
				perror("atg_pack_element");
				atg_free_element(row);
				break;
			}
			atg_box *rb=row->elem.box;
			if(!rb)
			{
				fprintf(stderr, "row->elem.box==NULL\n");
				break;
			}
			int val=*(unsigned int *)((char *)&types[IB_i]+b_stat_rows[i].v_off);
			val=(val+b_stat_rows[i].v_shift)*b_stat_rows[i].v_scale;
			if(i==STAT_COST&&types[IB_i].broughton&&!datebefore(state->now, event[EVENT_BROUGHTON]))
				val=(val*2)/3;
			atg_element *s_name=atg_create_element_label(b_stat_rows[i].name, 12, (atg_colour){0, 0, 0, ATG_ALPHA_OPAQUE});
			if(s_name)
			{
				s_name->w=100+(b_stat_rows[i].unit_first?0:8);
				if(atg_pack_element(rb, s_name))
				{
					perror("atg_pack_element");
					atg_free_element(s_name);
					break;
				}
			}
			if(b_stat_rows[i].unit_first)
			{
				atg_element *s_unit=atg_create_element_label(b_stat_rows[i].unit, 12, (atg_colour){0, 0, 0, ATG_ALPHA_OPAQUE});
				if(s_unit)
				{
					s_unit->w=8;
					if(atg_pack_element(rb, s_unit))
					{
						perror("atg_pack_element");
						atg_free_element(s_unit);
						break;
					}
				}
			}
			snprintf(b_stat_rows[i].value_buf, 6, "%5d", val);
			atg_element *s_value=atg_create_element_label(b_stat_rows[i].value_buf, 12, (atg_colour){0, 0, 0, ATG_ALPHA_OPAQUE});
			if(s_value)
			{
				s_value->w=40+(b_stat_rows[i].unit_first?32:0);
				if(atg_pack_element(rb, s_value))
				{
					perror("atg_pack_element");
					atg_free_element(s_value);
					break;
				}
			}
			if(!b_stat_rows[i].unit_first)
			{
				atg_element *s_unit=atg_create_element_label(b_stat_rows[i].unit, 12, (atg_colour){0, 0, 0, ATG_ALPHA_OPAQUE});
				if(s_unit)
				{
					s_unit->w=32;
					if(atg_pack_element(rb, s_unit))
					{
						perror("atg_pack_element");
						atg_free_element(s_unit);
						break;
					}
				}
			}
			SDL_Surface *bar=SDL_CreateRGBSurface(SDL_HWSURFACE, 102, 14, 24, 0xff0000, 0xff00, 0xff, 0);
			if(!bar)
			{
				fprintf(stderr, "bar: SDL_CreateRGBSurface: %s\n", SDL_GetError());
				break;
			}
			SDL_FillRect(bar, &(SDL_Rect){0, 0, bar->w, bar->h}, SDL_MapRGB(bar->format, 191, 191, 191));
			SDL_FillRect(bar, &(SDL_Rect){1, 1, bar->w-2, bar->h-2}, SDL_MapRGB(bar->format, 0, 0, 23));
			int bar_x=(val-b_stat_rows[i].bar_min)*100.0/(b_stat_rows[i].bar_max-b_stat_rows[i].bar_min);
			clamp(bar_x, 0, 100);
			int dx=bar_x;
			if(b_stat_rows[i].bar_rev)
				dx=100-dx;
			unsigned int r=255-(dx*2.55), g=dx*2.55;
			SDL_FillRect(bar, &(SDL_Rect){1, 1, bar_x, bar->h-2}, SDL_MapRGB(bar->format, r, g, 0));
			atg_element *s_bar=atg_create_element_image(bar);
			SDL_FreeSurface(bar);
			if(!s_bar)
			{
				break;
			}
			else
			{
				if(atg_pack_element(rb, s_bar))
				{
					perror("atg_pack_element");
					atg_free_element(s_bar);
					break;
				}
			}
		}
	}
}