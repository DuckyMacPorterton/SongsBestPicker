I wanted to figure out what my top 100 songs are, but I realized that sometimes I like, "Friend of the Devil," more than, "Alice's Restaurant," and sometimes it's the other way.

I decided the only way to really know what my favorite songs are is to do it empiracally.

You enter your songs (files must be locally accessible).  This randomly groups then into pods of 5 songs and plays them for you.

You put them in order from 1 to 5, favorite to least favorite.

This treats those as head-to-head results.  So, 1st place song beat 2nd, 3rd, etc, so has 4 wins and 0 losses.  

3rd place song lost to #1 and #2, beat #4 and #5, so has 2 wins and 2 losses.

This runs those through a modified ELO rating algorithm that includes a Margin of Victory adjustment, and builds song ratings for each song.

Over time, you'll get interesting song ratings that sometimes surprise you.

![Glorious main window](https://github.com/DuckyMacPorterton/SongsBestPicker/blob/master/Screenshots/Screenshot1.PNG?raw=true)

The absurdly awful name for this project ("SongsBestPicker") came because I knew when I looked for it in my directory of projects, "Songs", would be what I was thinking.  So I started it with Songs and the rest of that ever-so-poetic name justed flowed out, like sewage from an overturned port-a-john.

This project is also an experiment for me to pull all data from the database as needed and avoid keeping data in RAM.  It's not something I've done on a big scale before.
