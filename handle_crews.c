/*
	harris - a strategy game
	Copyright (C) 2012-2014 Edward Cree

	licensed under GPLv3+ - see top of harris.c for details

	handle_crews: screen for training, crew assignments etc.
*/

#include <math.h>

#include "handle_crews.h"
#include "ui.h"
#include "globals.h"
#include "bits.h"
#include "render.h"

#define HCTBL_BG_COLOUR	(atg_colour){23, 23, 31, ATG_ALPHA_OPAQUE}

atg_element *handle_crews_box;

atg_element *HC_cont, *HC_full;
char *HC_count[CREW_CLASSES][CREW_STATUSES];
SDL_Surface *HC_skill[CREW_CLASSES][CREW_STATUSES];

void update_crews(game *state);

int handle_crews_create(void)
{
	handle_crews_box=atg_create_element_box(ATG_BOX_PACK_VERTICAL, GAME_BG_COLOUR);
	if(!handle_crews_box)
	{
		fprintf(stderr, "atg_create_element_box failed\n");
		return(1);
	}
	atg_element *title=atg_create_element_label("HARRIS: Crews & Training", 12, (atg_colour){239, 239, 239, ATG_ALPHA_OPAQUE});
	if(!title)
	{
		fprintf(stderr, "atg_create_element_label failed\n");
		return(1);
	}
	if(atg_ebox_pack(handle_crews_box, title))
	{
		perror("atg_ebox_pack");
		return(1);
	}
	atg_element *top_box=atg_create_element_box(ATG_BOX_PACK_HORIZONTAL, GAME_BG_COLOUR);
	if(!top_box)
	{
		fprintf(stderr, "atg_create_element_box failed\n");
		return(1);
	}
	if(atg_ebox_pack(handle_crews_box, top_box))
	{
		perror("atg_ebox_pack");
		return(1);
	}
	if(!(HC_cont=atg_create_element_button("Continue", (atg_colour){239, 239, 239, ATG_ALPHA_OPAQUE}, (atg_colour){47, 47, 47, ATG_ALPHA_OPAQUE})))
	{
		fprintf(stderr, "atg_create_element_button failed\n");
		return(1);
	}
	if(atg_ebox_pack(top_box, HC_cont))
	{
		perror("atg_ebox_pack");
		return(1);
	}
	HC_full=atg_create_element_image(fullbtn);
	if(!HC_full)
	{
		fprintf(stderr, "atg_create_element_image failed\n");
		return(1);
	}
	HC_full->clickable=true;
	if(atg_ebox_pack(top_box, HC_full))
	{
		perror("atg_ebox_pack");
		return(1);
	}
	atg_element *head_box=atg_create_element_box(ATG_BOX_PACK_HORIZONTAL, HCTBL_BG_COLOUR);
	if(!head_box)
	{
		fprintf(stderr, "atg_create_element_box failed\n");
		return(1);
	}
	if(atg_ebox_pack(handle_crews_box, head_box))
	{
		perror("atg_ebox_pack");
		return(1);
	}
	atg_element *class_header=atg_create_element_label("Position", 12, (atg_colour){223, 223, 223, ATG_ALPHA_OPAQUE});
	if(!class_header)
	{
		fprintf(stderr, "atg_create_element_label failed\n");
		return(1);
	}
	class_header->w=80;
	if(atg_ebox_pack(head_box, class_header))
	{
		perror("atg_ebox_pack");
		return(1);
	}
	for(unsigned int status=0;status<CREW_STATUSES;status++)
	{
		atg_element *header=atg_create_element_label(cstatuses[status], 12, (atg_colour){191, 191, 191, ATG_ALPHA_OPAQUE});
		if(!header)
		{
			fprintf(stderr, "atg_create_element_label failed\n");
			return(1);
		}
		header->w=200;
		if(atg_ebox_pack(head_box, header))
		{
			perror("atg_ebox_pack");
			return(1);
		}
	}
	for(unsigned int i=0;i<CREW_CLASSES;i++)
	{
		atg_element *row_box=atg_create_element_box(ATG_BOX_PACK_HORIZONTAL, HCTBL_BG_COLOUR);
		if(!row_box)
		{
			fprintf(stderr, "atg_create_element_box failed\n");
			return(1);
		}
		if(atg_ebox_pack(handle_crews_box, row_box))
		{
			perror("atg_ebox_pack");
			return(1);
		}
		atg_element *class_label=atg_create_element_label(cclasses[i].name, 12, (atg_colour){207, 207, 207, ATG_ALPHA_OPAQUE});
		if(!class_label)
		{
			fprintf(stderr, "atg_create_element_label failed\n");
			return(1);
		}
		class_label->w=80;
		class_label->h=20;
		if(atg_ebox_pack(row_box, class_label))
		{
			perror("atg_ebox_pack");
			return(1);
		}
		for(unsigned int status=0;status<CREW_STATUSES;status++)
		{
			atg_element *cell_box=atg_create_element_box(ATG_BOX_PACK_HORIZONTAL, HCTBL_BG_COLOUR);
			if(!cell_box)
			{
				fprintf(stderr, "atg_create_element_box failed\n");
				return(1);
			}
			cell_box->w=200;
			if(atg_ebox_pack(row_box, cell_box))
			{
				perror("atg_ebox_pack");
				return(1);
			}
			HC_count[i][status]=malloc(32);
			if(!HC_count[i][status])
			{
				perror("malloc");
				return(1);
			}
			strcpy(HC_count[i][status], "count");
			atg_element *count=atg_create_element_label_nocopy(HC_count[i][status], 12, (atg_colour){191, 191, 159, ATG_ALPHA_OPAQUE});
			if(!count)
			{
				fprintf(stderr, "atg_create_element_label failed\n");
				return(1);
			}
			count->w=80;
			if(atg_ebox_pack(cell_box, count))
			{
				perror("atg_ebox_pack");
				return(1);
			}
			HC_skill[i][status]=SDL_CreateRGBSurface(SDL_HWSURFACE, 101, 17, 32,  0xff000000, 0xff0000, 0xff00, 0xff);
			if(!HC_skill[i][status])
			{
				fprintf(stderr, "HC_skill[][]=SDL_CreateRGBSurface: %s\n", SDL_GetError());
				return(1);
			}
			atg_element *skill=atg_create_element_image(HC_skill[i][status]);
			if(!skill)
			{
				fprintf(stderr, "atg_create_element_image failed\n");
				return(1);
			}
			if(atg_ebox_pack(cell_box, skill))
			{
				perror("atg_ebox_pack");
				return(1);
			}
		}
	}
	return(0);
}

screen_id handle_crews_screen(atg_canvas *canvas, __attribute__((unused)) game *state)
{
	atg_event e;

	while(1)
	{
		update_crews(state);
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
							return(SCRN_CONTROL);
					}
				break;
				case ATG_EV_CLICK:;
					atg_ev_click c=e.event.click;
					if(c.e==HC_full)
					{
						fullscreen=!fullscreen;
						atg_setopts_canvas(canvas, fullscreen?SDL_FULLSCREEN:SDL_RESIZABLE);
					}
					else
					{
						fprintf(stderr, "Clicked on unknown clickable!\n");
					}
				break;
				case ATG_EV_TRIGGER:;
					atg_ev_trigger trigger=e.event.trigger;
					if(trigger.e==HC_cont)
					{
						return(SCRN_CONTROL);
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

void handle_crews_free(void)
{
	atg_free_element(handle_crews_box);
}

void update_crews(game *state)
{
	unsigned int count[CREW_CLASSES][CREW_STATUSES];
	unsigned int dens[CREW_CLASSES][CREW_STATUSES][101];
	for(unsigned int i=0;i<CREW_CLASSES;i++)
		for(unsigned int j=0;j<CREW_STATUSES;j++)
		{
			count[i][j]=0;
			for(unsigned int k=0;k<101;k++)
				dens[i][j][k]=0;
		}
	for(unsigned int i=0;i<state->ncrews;i++)
	{
		unsigned int cls=state->crews[i].class, sta=state->crews[i].status;
		count[cls][sta]++;
		unsigned int skill=floor(state->crews[i].skill);
		dens[cls][sta][min(skill, 100)]++;
	}
	for(unsigned int i=0;i<CREW_CLASSES;i++)
		for(unsigned int j=0;j<CREW_STATUSES;j++)
		{
			unsigned int mxd=0;
			for(unsigned int k=0;k<101;k++)
				mxd=max(dens[i][j][k], mxd);
			snprintf(HC_count[i][j], 32, "%u", count[i][j]);
			SDL_FillRect(HC_skill[i][j], &(SDL_Rect){0, 0, HC_skill[i][j]->w, HC_skill[i][j]->h}, SDL_MapRGB(HC_skill[i][j]->format, 7, 7, 15));
			line(HC_skill[i][j], 50, 0, 50, 16, (atg_colour){23, 23, 23, ATG_ALPHA_OPAQUE});
			line(HC_skill[i][j], 25, 0, 25, 16, (atg_colour){15, 15, 23, ATG_ALPHA_OPAQUE});
			line(HC_skill[i][j], 75, 0, 75, 16, (atg_colour){15, 15, 23, ATG_ALPHA_OPAQUE});
			if(!count[i][j]) continue;
			for(unsigned int k=0;k<101;k++)
			{
				double frac=dens[i][j][k]/(double)mxd;
				unsigned int br=63+ceil(frac*192);
				line(HC_skill[i][j], k, 8-ceil(frac*8), k, 8+ceil(frac*8), (atg_colour){br, br, br, ATG_ALPHA_OPAQUE});
			}
		}
}
