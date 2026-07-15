subject ae823474a24261e92266df611c62a5f6007c33d3 fix(zc): waitdraw not working for ffcs over 32
subject c8e2c861edc73de3ff851d3cbc9bc8d07037d007 fix(zc): npc scripts not running on screen load
subject 935757d42efb0f999d7a5a1b0d2e893cac962614 fix(zc): enemies not running scripts on first frame with scriptloadenemies
subject 95e3288ec29c716b8645a2e01da311a94fded6b5 fix(zc): Test Mode issues when quitting during opening wipe
subject 9bf02571c6c09c182561bd565fc13fe701d4ad04 misc: ./zelda -load-and-quit `<qstpath>`
subject 5641cf8c1a9e76ed8fca68e7189f71c5efd20e1e fix(zc): new respawn points with ladder
subject 9762141dbe1bd7461b5dee03a627f290c6ed322a fix(zc): various newer hero movement
subject 0da042ba3b238bba1b468c753f24cce71cefc2a4 fix(zscript): Negative Array rule issues
subject dabe7c091b89fa2e09217dfb63c652184b78f65e feat(vscode): create VS Code extension
subject 6ecf0775afeac108bb4f806b76f9c1835c0a98d1 feat(vscode): update vscode extension keywords
subject d82a0d8458b3029283385db04f1d935fd0dbce48 misc: add 'std_zh' scope to changelog generation
subject f7228536ad55c2fb4bb20d00bba8b8c06d7e67c2 feat(std_zh): add 'GetLevelSwitchState()'/'SetLevelSwitchState()' helper functions
subject 20e5b2415ba90cb1f172e408d1703b8ffa1ee800 eweapon Rocks no longer hardcoded to die on solids
subject 742115e11333df188997ce1dcf962246d1047d36 Use correct .sav path for -standalone mode
subject 383b5d34a6d33963e0a0caef3f6ff2c1c49d920b refactor(zq): Upgrade sprite data list dialog
subject c4a6d8b720e294688fa688828f276e2caf5c1584 refactor(zq): Clean up `Quest->Audio->MIDIs` lister
subject ec60344a6bbe40e152d15bd316d35c0426da1315 fix(zscript): Make `npcdata->Flags` a bool array, remove `->Flags2`
subject 35c95f6cc8bdaf374cfef3bf06a4370e573a9cae refactor(zq)!: Update door selector to zcgui
subject e5cf4d4dab7aa57e7d0d91e7eecfa03ae604349d feat(zq): Add descriptive labels and dropdowns to walker type attributes in enemy editor
subject d253cbb843b7652810c69b3e9d7044c1c136fef2 refactor(zscript): optimize sections of zasm that are run only for their side-effects

drop 165b42156443f963f41ce0f5a68638010ff7c137 feat: setting for overlay subscreens to hide during message strings

squash 2571479f44d6a99c06e097e7c6a34378d4e14e28 refactor(launcher): remove title option
squash 17e56f1410dc0bd04183ba13eda55e4892979b7f fix(zc): put title.mid back in zelda.dat, since it is available to quests
squash d6878d6d2a7cffa96ba3090ac29c7b2f5bf758d7 refactor(zc): remove title option from menu
squash dcd117a1a9d4c9aefd653f5a56f090ad231d394e misc(zc): remove unused title/logo assets from classic_zelda.dat
squash bcfd8bbf59023ddc0ca8db77b260dadd6e8504b3 refactor(zq): remove defunct -notitle switch from quest packager
pick e4640ab770d6aae79df2a2748646edd8d6fb63ac refactor(zc): Remove the original Zelda title screen

drop 93953beea071045757d2d7b69b053149b3e2fc21 chore: fix last commit
drop 3d944ba6532d7fb96ed6eb6e78463c71add6775e build: fix windows build

squash ad58eb0ab7b2aec8689bfd29af3cd5f3bc52513f feat(launcher): re-open launcher after update
squash e11f6058b607fca5fcf6e951d3305537c95009fe test: re-enable updater test
pick 5b3162034226d0a7d0e00679df0080eb0fd5d16e feat: add software updater

drop 6e0ff07c5897ec6900e126d6b4bcf928a8fd9b13 chore: fix previous commit
drop 9f656d8abb1d08b276448a92dbbe605a93e9fd6a build: fix build for msvc

squash 593beae6594ff734d535b3664dd867b06bbb6305 test: move zquest save classic_1st.qst test to python
pick 7c8a5625c582a12b7d779cf6173f670543d25214 test: move zquest save classic_1st.qst test to python

drop 2769fc99965e764f2b2f809ea935bb15d5db4213 build: fix windows compile
# ! accidentally duplicated this message
drop 774573f246c6ab49609f653198d7e3b11597fe79 feat(zq): implicitly add `<qst dir>/scripts` to include paths

subject 3c86a76b8d93c250bcf636ce19c3206be79b94c9 fix(zscript): 'delete' within a destructor being buggy
subject d188ece0e85762c8238e4081951da877a0b458e9 feat: option to disable transparent darkrooms stacking multiple transparent layers

drop 287e74d84200c3abdd7f1750207a28200531a09d fix(zq): reenable floating selection for dense forest

reword 88d94ba3c1917b36a3801e2d93a00f5f80554ec4 feat(zq): -smart-assign CLI switch to compile + assign
Example usage: ./zeditor.exe -smart-assign your_quest.qst
=end

subject 86b88dff9c410348fc888a312d68745bda857b2e feat(zc): Show bottom 8 pixels by default for most quests made prior to 2.55.9
subject cf2769fe95ae01ba07d2a028c803e284fdecc6a8 feat(zscript): Arrays now are separately typed from non-arrays

drop ca846de53dca7814a112b604d34bc4f9df721a06 fix(zq): Update cambria tileset
drop 6406f4745d9317efa80d8fafce24b3ba84c3cae1 fix(zq): update Cambria tileset
drop 99fedb984d3bbb1b2e49bdd968e873712e78285d fix(zq)<: Update cambria tileset

# ! trigger examples

drop 4a50b8a0167d41f5425e6a8cf233d3a99b9cba4e misc: update `quests/examples/trigger_example.qst`
drop 73f5b12e58e684cc1c066729639f981506524210 misc: update `trigger_example.qst`
pick 2e886e3e1473be779843e5055d3a723c7d7ba88b feat(zq): Add `quests/examples/trigger_example.qst`

# ! don't hide
reword 1b4fd9b5077b6970da14f11fe594f9edfa7e8674 fix(zc): armos/graves not activating properly from Large Hitbox player
=end

drop 8ec6c7c39922b3f8f348d893b697783776ff9f4d fix(zscript): prevent var declaration having wrong flags in ASTDataDecl
drop 39098a4a1dca13bf5d7dd467d72016a50975c73d fix(zc)!: handle older replays in replay_get_zc_version_created
drop 236e4bc8396602c1101a7cb5c0b0bc1262b0122f fix(zq): screen enemy list dialog constantly resetting focus
drop d82e808320a33d6daa957231866383c23122118b fix(zq): item advanced paste not pasting item name properly
drop ab9ad2fc84b5ba56b2cac094ef962fba063057ad fix: prevent rare crash when destroying null midi
drop 209b276703b4418fef1f33f2c01142e392497aeb fix(zq): "Follow Warp" not respecting dmap xoffset
drop 7ff9ca9c9621ba566062020ae358bb4d2763dd70 fix(zscript): improve unsafe code in STARTDESTRUCTOR/ZCLASS_MARK_TYPE commands
drop 8fdcbfe65c1c911de2d445d13fb5d2dac8b37053 fix(zq): typo in epilepsy qr helptext
drop 842d2a0bcb0eafde8550c6a36ce658a53657043b fix(zq): layer dialog not marking quest as "needing to be saved"
drop 44df6116058a4acfd3910e9bdcb208c618b1add0 fix: save menus not stopping sfx before opening (ex. ocean sounds)
drop f7a7e2a825ae5927a7ed3cb8b64c945da49b77de feat(web): add zasm output to zscript playground
drop 3ea93b743739fb402ff4c3524d61d74e74bd596d misc(zscript): re-number zasm comment numberings
drop 0faa3c2b6d1e1aa3aed7a9012edc7a2f5143d3fa feat(zc): remove dead ZASM

# ! obsoluted
drop 4be7097b035eff3e44a75370f25a9587f2d5b28b refactor(zscript): upgrade to jitted script without stalling game

# ! various fixes of bugs only introduced in 3.0 prerelease builds
drop 1782559d7e3bd63725c069e1411f453ca75fac9f fix(zc): Pound combos on layers changing the wrong layer's combo
drop 664bedf36632237ec9fb886509c49de3ade0e88c fix(zc): combo trigger conditionals for dark/not dark no longer inverted
drop a3cd7723bf674b9770d54253d2282e697a7a9579 fix(zc): Push (Right, Many) only pushing once
drop cb07f7ca5d6c40b2551ea954018d61de1c7d95a9 fix(zc): Icy Floor collision in sideview
drop 97e1435e81a9ef8ede7c614e876395a343fd9af9 fix(zq): add missing moveflags to enemy editor
drop 49fcf283f445ed496257f04218e25a7c4c3198d8 fix(zq): square misalignment in zoomed-out expanded mode
drop 02bc0f4ffba35b6bd81fbdc3e783f2574b543885 fix(zscript): a couple of small errors in recent optimizations
drop c043aaae32f611d5b74c05ebfcc9b2a1ca4beff5 fix(zq): next-screen-preview visual bugs re: zoom out
drop 2f845b504556032a00ac0c0a181a04a210f39b7f fix(zscript): fix many internal functions
drop 0903ecdc18ec6b1a7542e287218988928b497839 fix(zq): Screen Enemies Dialog not setting "Dirty" State for saving
drop ab5f763155939f2025ca8980084bb86404c2dd09 fix(zq): enemy editor spawn/death sprites swapped
drop 2d2c5de928d38512a80bd8b079ff3de3983017a1 fix(zq): combo alias cursor visual issues
drop aa467263cdf9d92eaaf7ab6e2ca4a8fee54ccf17 misc(zc): add -analyze-zasm-duplication for finding duplicated functions
drop 1152e5548ac155eba7ea08ff410d7b4e3b130319 fix(zc): Region 'Physical' mapping option not working outside of regions
drop 6be636e6d7c83d294de1c73019c29b97670bd537 fix(zq): description of 'Consume w/o trig' triggerflag was misleading
drop 41139dde14d5cfa964c555695f478feb320c9dd0 fix: use correct app id in zalleg_setup_allegro
drop 35505498d5a8e2c2668eb7a1d21715ed9af3900a fix(zq): add missing sideview ladder flags to top row of item cellar and passageway
drop 7f249ee2ec123fefdbc69b968d8f99fe71bf5c82 fix(zscript): fix broken array literals when declared as array pointers
drop 8a3f7b5de7d814cdd8598fb3d8a9e72682f5e153 fix(zscript): internal label error in some cases
drop dc424156c65998ec48c39d83768815c01d90331e docs(zscript): clean up docs for some global functions
drop fafa5cba970453584766fcf419ef03fdf7e87cd0 fix: prevent rare crash related to loading fake weapon script

drop 14267e6b6fd29a990b66438ed35ccb9e79d9ce74 feat: subscreen minimap compass can point to boss instead of McGuffin
drop 9a75c5452b9837bf6ac84a0441728e9e513ff422 build: upgrade sentry to 0.9.0

drop d00a94dfb186dd0fb49d9cf68d6776cfca482ed3 build: update websocketpp to newer develop branch
drop ae375c53032c4ce491c70330d44c83ee138e3f1e fix(zscript): websocket pool being destroyed on 'cont_game()'
drop 764e539e0fb340d2e231448ee74ae5f9b5c852d1 refactor(zscript): document, clean up, and sanity check websocket stuff
drop ce2f07d9c5a535ca67fc935c114494f0f4f3734d refactor(zscript): document, clean up, and sanity check websocket stuff
drop 83df3a67bcf48288e64b429dcc2be5627301e01f fix(zscript): websocket double-deleting their arrays, doing extra alloc/dealloc work
drop 32996be02bd5d7eee2de4b86de266d00487b1692 fix(zscript): crash in UserDataContainer, broken `websocket->Free()`
drop f98c49c6966990aa76cb43a0f993634caa17fe19 fix(zc): websockets reading wrong pointer
drop a2801b774c7ec896b83cd0a623ef4aafca9952df build: fix websocket cmake project from breaking CMAKE_CONFIGURATION_TYPES
drop 26c78ebec25fcaab7db36521f0e4f3a894e9a8a0 fix(zscript): '@AlwaysRunEndpoint("off")' erroring despite being valid
drop a12dbe335c222b47601737b9e9b06051c35fa2f1 docs(zscript): update webdocs
drop f17fa5f6a1b5eae327e003b0e44bef0aac767c83 docs(zscript): use .. version*:: admonitions for '@version' docs
drop 33effdbf0d5b560b1758767ea27d246f6cdb4475 docs(zscript): add examples using `Screen->DrawOrigin`
drop 52d0aa35ad1087803e2250ed80d7ebf3a88ebdf5 misc(zc): temporarily disable compare zasm opt as I improve in a branch

drop ab128735140decc4e634aef7fd0732c9fd837bbc fix(zscript): prevent compiler crash when parsing long decimals
drop f318313547d88c43bf347afb31d39f6e4c76a709 fix(zq): visual bug with Lister dialog preview size
drop 5abdbac9d734fea41b50df77a1894484c0ccda3b fix(zq): Map Styles dialog not counting as a "change" that needs saving
drop a6d3ae9701173dd3b046d381832735539d54939e fix(zc): Heart Container / Magic Container cheats using outdated values
drop 9d50f2f92f34391f4cc4f1bd517a8205beaf2fa1 fix: prevent out-of-bounds write from invalid save menu count
drop 9ea87637b022559078de9b42e0a4f1222e4ebc05 fix(zc): harden replay assert against source dimension changes

# ! vs code extension

drop 5526c1ddc2511e64f9e97452606af15fd1f80986 misc(vscode): publish 1.0.8
drop c58661f071c6b6e0f09ceb4dc13cc667371401f4 misc(vscode): publish 1.0.9
drop 790dc584ff36aac6390de58d79410ae2bc85c307 misc(vscode): publish 1.0.10
drop 23981d26c925e8ca6affd213e60fec46754a39c6 misc(vscode): publish 1.0.11
drop 87e68c33c1b34fe8970415c8be595edea9b78545 misc(vscode): publish 1.0.12

# ! reverted
drop 636cd693ebcc1cbec50ced5af8ece8be31f04f64 fix(zc): only grab one held item at a time
drop 1a1bd37ac769db838a390e88b6fef03288576b01 fix: changer ffcs counting for trigger groups and similar effects (they shouldn't)

reword c4ff8de1a0e9aa5f2c9aeb7ebfef556d8ffc20d2 feat: Add many new SCCs
- CollectItem
- Delay
- ForceDelay
- KillHero
- Counter
- MaxCounter
- GoIfAnyScreenState
- GoIfGlobalState
- GoIfLevelItem
- GoIfLevelState
- GoIfScreenState
- GoIfSecrets
- GoIfScreenExState
- GoIfAnyScreenExState
- SetGlobalState
- SetLevelItem
- SetLevelState
- SetScreenExState
- SetAnyScreenExState

See https://docs.zquestclassic.com/tutorials/message_strings for details.
=end

# ! drop CI stuff

drop 1a8cbf99daded2c578e1163a853831e0e92bbb9b ci: reset cache for web builds
drop 16ce632d59cc0e38f7ee2e6a397ddbb95a56a2c1 ci: for web tests, build for RelWithDebInfo
drop c0e0151c499e52c41af50a84f2c517971a1ddc4b ci: include coverage for all apps, not just zplayer
drop 75da202d66b1f3d3e1ad37ca44a6374756ad0a6e ci: bump build cache so 2.55 branch doesn't conflict
drop 2ebfaeffcf7b3118ba7e2efa6c17d6d127ac49ea ci: set correct test results folder for web replays for upload
drop 15b7ff4aa5da3829107662191d7444126e4040e8 ci: get database update cron working again
drop ec499225b80237aeeaa055ab0043e5229577c69b ci: fix type in download-artifact v4 action
drop e272d0914fd05db5944f2477cff73d66c33f8804 ci: fix another typo in download-artifact v4 action
drop bfd9daa06a00e56f506e14ed67087d3ffc9f90c1 ci: set test results folder names for jit/non-jit replay runs
drop bbb53680f956136f2c1417b4b1d5aff042ccd58b ci: fix replay report generation for forks
drop 12b1fa6541c20cea165e22259cb10d142721013d ci: upgrade to python 3.12.8
