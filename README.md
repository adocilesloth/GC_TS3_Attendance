Global Conflict Teamspeak3 Attendance Generator

Designed to get the names of people who attend GC events from army channels.
Takes attendance every 15 mins.
It uses TS3's client quiery plugin so this plugin must be running for this to 
work.

It generates three .txt docs:
	Channels.txt
		Holds all the channel tree cid numbers.
	Client.txt
		Holds a full, unsorted list of all clients in the tree when the program
		checks for clients.
		Will contain duplicate entries for clients if they are there for more
		than one check.
		Has \s instead of spaces in client names
	Attendance-yyyy-mm-dd.txt
		Has the sorted list of all clients in the tree when the program runs.
		Each client (should) only appear once. It takes into account suffixes
		for number of rounds played.

If it's the first time the program is run in a campaign, or a new channel is
added to the army's channel tree, setup needs to be run to create Channels.txt.

There is a chance this program may be tempremental. If it is, send me (A Docile
Sloth) a PM with how it's screwed up and I'll see if I can fix it.