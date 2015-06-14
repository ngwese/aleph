# release (bees)

building a release involves a few steps not need during normal
development. the purpose of this process is to ensure the build is
easily repeatable, roughly speaking this entails:

- cloning a fresh copy of the source

- building all apps and modules

- installing onto a clean SD card

- testing

- updating change notes, merging into master

this process could be automated if there sufficient desire.

the steps below assume one has contributor priviledges to the master
repository `https://github.com/tehn/alpeh`.

## prepare

using an existing clone of the source tree is possible however it good
practice to use a fresh clone to ensure everything has been correctly
added to the repository.

clone the master repository:
```
git clone https://github.com/tehn/aleph aleph-release
cd aleph-release
git checkout dev
```

using a branch specifically for assembling the release helps keep
changes to version numbers and release notes off of the `dev` and
`master` branches while assembling and testing.

create a `release/bees-0.6.1` branch off of dev:
```
git checkout -b release/bees-0.6.1 dev
```

## build

on the release branch bump any version numbers which have not yet been
updated by editing the `version.mk` for the app(s) or modules(s) which
have ben modified. breaking changes should result in a minor version
increase (see [ http://semver.org/ ]).

for example bumping bees to 0.6.1:
```
$EDITOR apps/bees/version.mk
---------------------
| maj = 0
| min = 6    <- increase for any breaking change
| rev = 1    <- increase for fixes
|
git add apps/bees/version.mk
git commit -m "bees: bumped version to 0.6.1"
```

### building bees

```
cd apps/bees
# this makes a debug build
make clean
make
# this makes a release build
make clean
make R=1
```

### building modules

for each of the lines, waves, and dsyn modules:
```
cd modules/<module_name>
make clean
make deploy
```

## install

in general a new release can be installed on a sd card with an
existing release however it is recommended that the card be erased
first in order to ensure the new release is complete (MS-DOS FAT
format, volumne name ALEPH).

```
./install.sh /path/to/sdcard/ALEPH
```

at the same time we will prepare a tarball for this release as well:
```
mkdir -pv dist/aleph-20150612
./install.sh ./dist/aleph-20150612
cd dist
tar czvf aleph-20150612.tar.gz aleph-20150612
```

## test

flash the aleph with the new (bees) release and make sure things seem
stable.

## finish

once the release is tested and things ready to go.

- update the `CHANGELOG` as apropriate; add and commit it to the
release branch

- merge the release branch into master
```
git checkout master
git pull --ff-only
git merge release/bees-0.6.1
```
if the `git pull --ff-only` operation fails then that means that
modifications to the `master` branch have been made in this clone
which need to be reverted.

- tag the release

```
git tag -a bees-0.6.1 master
```

- merge the tagged release back into `dev`

```
git checkout dev
git merge master
```

- upload the `aleph-2015####` tarball created previously to github.

