Playing a Quest
===============

The recommended way to play quests is via the latest stable version of ZQuest Classic for your operating system.

#. `Download <https://zquestclassic.com/releases/>`_ the latest stable version of ZQuest Classic
#. `Search for a quest <https://www.purezc.net/index.php?page=quests>`_ you like, download it, and place it inside the ``quests`` folder in the ZQuest Classic installation folder
#. Open zplayer, the client for ZQuest Classic quests

   * Windows: launch ``zplayer.exe``; or use ``zlauncher.exe`` then click "ZC Player"
   * Linux: launch ``zplayer``; or use ``zlauncher`` then click "ZC Player"
   * Mac: launch ``ZQuest Classic.app`` then click "ZC Player"

#. Use the arrow keys to navigate to ``NEW FILE``, press Enter, type in a name for save file, then select the ``.qst`` file you wish to play
#. Press Enter to start the game

The default keyboard mapping for buttons ``A`` and ``B`` are "Z" and "X" - to modify this, or see what keys the other buttons are mapped to, open the system menu (just click the mouse) and select ``Settings -> Controls -> Keyboard``.

Before closing the player, be sure to save. Some quests have special saving locations within the game world, but most can only be saved by opening the system menu (click the mouse), selecting ``Game -> End Game``, then choosing the "Save" option.

Alternative Method: play directly in the browser
------------------------------------------------

You can also search for and begin playing a quest directly in the `web version <https://web.zquestclassic.com/play/>`_. Open the ``Quest List`` at the top of the page, find a quest you like, then click the ``Play!`` link to start the game.

Note, the web version is still experimental, and may not work ideally in all browser or on older computers. For the most verified experience, use Chrome.

(For quest authors): Players can load your quest in the web version in a few different ways:

1. If your quest is uploaded to https://www.purezc.net/ and you've granted permission for the web version to load it, players can find it in the Quest List. You can also share a URL that will directly load it, like this: https://web.zquestclassic.com/play?open=quests/purezc/373&name=Links-Quest-for-the-Hookshot-2-Quest
2. If you don't want your quest uploaded to https://www.purezc.net/ (for example, it is a demo), but still want a direct URL so players can easily load your quest: we can manually upload your quest on our servers
3. If you want to host your qst file on your own server, you can have the web version load it like this: https://web.zquestclassic.com/play?open=https://www.example.com/game.qst (where the ``?open=`` value is the URL to your quest file). Your webserver will need to set this response header for ``Access-Control-Allow-Origin "*"``. Note: this currently does not support external music files
4. If you just want to give players your qst file directly, they can load it into the web version themselves. They will need to either attach a folder from their computer to the web version (not all browsers support this), or upload the file in the Settings tab. Once done, they can create a new save slot and navigate to the ``/local`` folder to locate the qst

For 1. and 2., contact Connor (discord connorclark#9300).
