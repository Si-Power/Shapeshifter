SHAPESHIFTER

This is a TWHL Half-Life 1 SDK v2.3 engine modification (incomplete yet), that allows player to turn on controllable/drivable monster via console command.
Mod heavily inspired by another mod: Rocket Crowbar v1.9 (by Laser, (c) Derek Hageman 1999-2001), that had very interesting feature to transform players into ingame NPCs with various abilities.
This mod is a reverse engineering attempt to resurrect transform feature for competitive or cooperative gameplay.

At now only monster_human_assassin is implemented.

New console commands are available:

	play_hassassin <player_index> - transform player with certain index into monster_human_assassin
		For example: play_hassassin 1 - transform player 1 into monster_human_assassin 
		(Player's ID in singleplayer mode always equals 1)

	play_release <player_index> - release player with certain index from transform mode
		For example: play_release 1 - release player 1 from transform mode
		(Player's ID in singleplayer mode always equals 1)
	
	debug_traceline 1/0 - toggle on/off rays visualization (only in NPC mode)

EDITING:
Can be also edited with Visual Studio 2022. Open the project.sln for editing and building the code.

INSTALLATION:
After building (or downloading) place the cl.dll and hl.dll files in the corresponding folders of the game.

Problems to solve:
1. Controllable NPC can't walk near walls, only stuck;
2. Controllable NPC bumps on invisible ceiling while jumping;
3. Controllable NPC can stuck before slopes, stairs and aligned planes, can't slide on them like player does;
4. Controllable NPC can only imitate native walking and running, but not use own schedule movement;
5. Need to increase memory limits of sounds and models since this mod must work on any map of Half-Life campaign with almost any monster in future;
6. Save/load or changing level while NPC drive causes player-camera fly far out from the level.








Half Life 1 SDK LICENSE
======================

Half Life 1 SDK Copyright© Valve Corp.  

THIS DOCUMENT DESCRIBES A CONTRACT BETWEEN YOU AND VALVE CORPORATION (“Valve”).  PLEASE READ IT BEFORE DOWNLOADING OR USING THE HALF LIFE 1 SDK (“SDK”). BY DOWNLOADING AND/OR USING THE SOURCE ENGINE SDK YOU ACCEPT THIS LICENSE. IF YOU DO NOT AGREE TO THE TERMS OF THIS LICENSE PLEASE DON’T DOWNLOAD OR USE THE SDK.

You may, free of charge, download and use the SDK to develop a modified Valve game running on the Half-Life engine.  You may distribute your modified Valve game in source and object code form, but only for free. Terms of use for Valve games are found in the Steam Subscriber Agreement located here: http://store.steampowered.com/subscriber_agreement/ 

You may copy, modify, and distribute the SDK and any modifications you make to the SDK in source and object code form, but only for free.  Any distribution of this SDK must include this license.txt and third_party_licenses.txt.  
 
Any distribution of the SDK or a substantial portion of the SDK must include the above copyright notice and the following: 

DISCLAIMER OF WARRANTIES.  THE SOURCE SDK AND ANY OTHER MATERIAL DOWNLOADED BY LICENSEE IS PROVIDED “AS IS”.  VALVE AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES WITH RESPECT TO THE SDK, EITHER EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, TITLE AND FITNESS FOR A PARTICULAR PURPOSE.  

LIMITATION OF LIABILITY.  IN NO EVENT SHALL VALVE OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR INABILITY TO USE THE ENGINE AND/OR THE SDK, EVEN IF VALVE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.  
 
 
If you would like to use the SDK for a commercial purpose, please contact Valve at sourceengine@valvesoftware.com.


Half-Life 1
======================

This is the Half-Life SDK, version 2.3, upgraded to compile under Visual Studio 2019.

This SDK is provided as-is, with no further support provided.

It is highly recommended to use the newer SDK provided by Valve here: https://github.com/ValveSoftware/halflife

This SDK was designed for WON Half-Life and is not guaranteed to work with Steam Half-Life.

Compiled dlls are placed in the `repository root/Debug` and `repository root/Release` directories. Place these in your mod directory to run them.

Basic testing shows this SDK does work with both WON version 1.1.0.8 and Steam Half-Life, but it is not guaranteed to remain compatible with Steam Half-Life in the future.
