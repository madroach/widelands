push_textdomain("world")

dirname = path.dirname(__file__)

include(dirname .. "../terrain_affinity.lua")

wl.Descriptions():new_immovable_type{
   name = "aspen_summer_pole",
   descname = _("Aspen (Pole)"),
   size = "small",
   animation_directory = dirname,
   terrain_affinity = terrain_affinity,
   programs = {
      main = {
         "animate=idle duration:38s",
         "remove=chance:18.36%",
         "grow=aspen_summer_mature",
      },
   },
   spritesheets = {
      idle = {
         basename = "pole",
         fps = 8,
         frames = 4,
         rows = 2,
         columns = 2,
         hotspot = { 12, 28 }
      }
   },
}

pop_textdomain()
