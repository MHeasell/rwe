---
layout: default
title: Frequently Asked Questions
---

#### How does RWE compare to Spring?

The [Spring RTS Engine](https://springrts.com/),
originally called TA Spring, is an open-source real-time strategy game engine
that started life as a new, fully 3D engine that understood TA data files.
Though Spring started with roots in TA, the project made many deviations
from TA-like gameplay and focused on enhancements like deformable terrain,
dynamic water rendering and lua scripting support.
Spring never achieved full compatibility with original TA mods,
and various "Spring-only" mods were created using the new features,
eventually including mods comprised of entirely original content.
Later, the Spring developers dropped "TA" from the name
and presented the project as a standalone game engine.

RWE is different because the primary mission is to be compatible with TA data,
without modification,
and to provide a look and feel that closely matches TA.
The project also specifically commits to prioritising core features and compatibility
over new enhancements and effects.

#### How does RWE compare to TA3D?

[TA3D](http://ta3d.org/) was a project whose aim was to
create a new, replacement engine for TA, but in full 3D.
The project was seen as an alternative to Spring
that would focus on a TA-like look and feel
but with modern enhancements.
TA3D made substantial progress but unfortunately development stopped
before the project reached a playable standard.

TA3D was a very ambitious project that attempted to implement
a vast number of enhancements over TA.
These included new graphical effects such as water ripples and waves,
a fully 3D world including new, high definition art assets,
new pathfinding techniques and more.
However, work on these enhancements was prioritised over work on core features
and compatibility, which prevented the project from reaching its goal.

TA3D had a very similar objective to RWE,
and before starting RWE I attempted to revive TA3D
with the goal of bringing the project to a playable standard.
However I found that the TA3D codebase contained numerous fundamental design flaws
that would prevent the project from achieving its goal,
and after six months of work I concluded that the project could not be saved.
I made the decision to start from scratch on a new, entirely original engine,
with a commitment to avoiding the same mistakes.

RWE is different from TA3D because RWE has a smaller mission than TA3D
(just be like TA, no full 3D)
and has a specific commitment to prioritising core features over new enhancements.
In addition, the architecture is carefully designed
to ensure that the project will be able to achieve its goals
and grow in a sustainable way.

#### How will this project succeed where TA3D failed?

I offer no guarantee that I won't just stop working on this one day, for whatever reason.
I am doing this for free, after all.
However, my project is more likely to succeed for the following reasons:

* Clear focus on core features.
  There will be no exotic water shaders or scripting language support in RWE,
  work will always be towards supporting features that TA supports.
* Industry/technology advancements.
  Twenty years on, software design is more well understood by the industry.
  Similarly, programming languages and tools have also improved (e.g. c++11, c++14, c++17).
  Hardware has also advanced such that big performance issues twenty years ago are trivial concerns today.
  These advancements make it easier for a single person to build and maintain more complex systems.
* Personal skills. I am a professional software developer with a lot of knowledge and experience, so I know how to solve a lot of problems.

To elaborate on the first point, implementation is guided by the following rules:

1. Be compatible with TA data
2. When given TA data, behave like TA does
3. If there's an alternative that's both easier to implement and at least as good as what TA does, do that instead
4. Don't do anything else

I aim to post a progress update at least once a month that explains what has been done and what the current goal is.

#### Will you implement feature X?

Following the principles above,
if the feature forms part of TA's multiplayer or skirmish gameplay,
the answer is probably yes.
Campaigns are not currently being considered,
and the same goes for features and enhancements added by the demo recorder,
unofficial patch and others.
However, these might make it in once the basic features are completed to a high standard.

If the feature is from Spring, Supreme Commander or another game and is not in TA, the answer is no.

#### Will you fix bug X from TA?

Bugs are a delicate issue.
On the one hand, bugs are incorrect or annoying behaviour
that cause problems or detract from the experience.
On the other hand, some bugs have an impact on gameplay that players rely on.
Fixing them changes the balance of the game and is in conflict with the engine's goal
of behaving like TA does.

RWE tries to take a pragmatic approach, evaluating bugs on a case by case basis
with a view to fixing them unless there is a compelling reason not to.

There are also bugs whose root cause is not attributed to the engine,
but to unit scripts or data errors.
These bugs are not considered by RWE, as RWE is purely an engine.
However these bugs can still be addressed by mod makers just like in TA.

The following behaviours in the TA engine are considered bugs
and are either fixed in RWE, will be fixed when RWE implements that part
or are not an issue in RWE:

* In TA, units will not aim directly at unit sweetspots,
  but instead at a different, unexpected location.
  In RWE, units aim directly at the sweetspot.
* In TA, in multiplayer games, sometimes AI units will be untargettable.
  This happens because of a bug when setting up teams in the multiplayer lobby.
  RWE does not implement the multiplayer lobby --
  this is provided by the RWE launcher, which does not have this issue.

The following are not fixed in RWE:

* "Sparking" -- the practice of damaging an activatable structure (e.g. moho metal maker)
  whilst it is being built, to cause it to activate before completion.
  This is a script bug.
* Missile launchers, lasers being unable to hit the ARM Pelican while it is in the water.
  This happens because the unit's sweetspot is too low.
