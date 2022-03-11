#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_primitives.h>

#  include <allegro.h>

void abort_example(char const *format, ...)
{
  va_list args;
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  exit(1);
}

int main(int argc, char **argv)
{
  ALLEGRO_EVENT event;
  bool have_touch_input;
  bool mdown = false;

  if (!al_init())
  {
    abort_example("Could not init Allegro.\n");
  }

  // just checking that allegro-legacy works
  packfile_password("lol");

  if (!al_init_primitives_addon())
  {
    abort_example("Could not init primitives.\n");
  }
  al_init_font_addon();
  al_install_mouse();
  al_install_keyboard();
  have_touch_input = al_install_touch_input();

  ALLEGRO_DISPLAY *display = al_create_display(800, 600);
  if (!display)
  {
    abort_example("Error creating display\n");
  }

  ALLEGRO_FONT *font = al_create_builtin_font();
  if (!font)
  {
    abort_example("Error creating builtin font\n");
  }

  ALLEGRO_BITMAP *bitmap = al_create_bitmap(800, 600);

  ALLEGRO_EVENT_QUEUE *queue = al_create_event_queue();
  al_register_event_source(queue, al_get_keyboard_event_source());
  al_register_event_source(queue, al_get_mouse_event_source());
  al_register_event_source(queue, al_get_display_event_source(display));
  if (have_touch_input)
  {
    al_register_event_source(queue, al_get_touch_input_event_source());
  }

  for (;;)
  {
    if (al_is_event_queue_empty(queue))
    {
      al_set_target_backbuffer(display);
      al_clear_to_color(al_map_rgba_f(1, 1, 0.9, 1));
      al_flip_display();
    }

    al_wait_for_event(queue, &event);
    if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
      break;
    if (event.type == ALLEGRO_EVENT_KEY_CHAR)
    {
      if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
        break;
      switch (toupper(event.keyboard.unichar))
      {
      case 'R':
        // TODO
        break;
      }
    }
  }

  al_destroy_display(display);

  return 0;
}
END_OF_MAIN()
