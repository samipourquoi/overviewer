## Overviewer

A real-time overviewer for the latest versions of Minecraft.

**⚠️ Work in progress! ⚠️**

---
### Structure
Couldn't figure out names to use for each directory, so I just gave them 
random fish names.

| Directory  | Purpose                                  |
| ---------- | ---------------------------------------- |
| `pilchard` | Individual chunk rendering               |
| `halibut`  | Individual chunks to whole map rendering |
| `salmon`   | Region file watcher and parser           |
| `herring`  | Web server and orchestrator              |
| `anchovy`  | Frontend map viewer                      |

### Build (outdated)

First clone the repository:
```shell script
$ git clone https://github.com/samipourquoi/overviewer.git
$ cd overviewer
```

To compile and run, enter the following commands:
```shell script
$ make
$ ./overviewer
```

---
Coded by myself, inspired my [Mapcrafter](https://github.com/mapcrafter/mapcrafter) 
and [Minecraft Overviewer](https://github.com/overviewer/Minecraft-Overviewer). 

Feel free to contact me on discord: `samipourquoi#9268`.