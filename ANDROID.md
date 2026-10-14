This document describes noteworthy specifics about DB48X (or DB50X) on Android.

The information here is based on development work done after the release of version 0.9.20.
Hopefully the changes, particularly [PR #1816](https://github.com/c3d/db48x/pull/1816),
will be incorporated in the next release.


# Use

DB48X comes with a set of configuration, help and (library) rpl program files.
You can add your own programs.
You can `Store` stack objects in a file and can `Recall` an object from a file to the stack.


## On your Android device

The location of these files on Android is `.../Android/data/org.db50x/files/`.
Unfortunately the Android `Files` App doesn't allow you to see or access that direcory
as is shown in the screenshot below.

Note that the screenshots here show a relevant state in the process.
At the top of the screenshot there are a few lines that show where this is in the file sturcture.
How you get there from activating the App, will require some navigation tapping.
That is not shown or described here but left as an exercise.

<img src="images/ANDROID.md/Files.png" width="40%">

You need an other File Manager App, like `Cx File Explorer`,
that does have permissions to access `.../Android/data/`.
Even `Cx File Explorer` is restricted.
It does show the contenct of  `.../Android/data/`.
It does allow to navigate to and to enter `.../Android/data/org.db50x`.
Then it can't go further as shown in the left screenshot below.
But it does provide a way out via `System File Manager`.
That does give you access to  `.../Android/data/org.db50x/files/` as shown in the right screenshot below..

<img src="images/ANDROID.md/Cx File Explorer.png" width="40%">
<img src="images/ANDROID.md/System File Manager.png" width="40%">

With the `System File Manager` App you can work on / with the DB48X files.
You can e.g. copy files to / from you Google Drive.
You could backup / restore your files.


## With a computer

Attach your Android device over USB to your computer.
With the appropriate settings you can then use your computers file manager to work with the files.
You could edit configuration files.
You could add your own rpl program files.
Last but not least, you could make a backup of anything you need to keep.

The partial screenshot below shows the use of the Dolphin file manager on linux.
From the root of the Android device, `Pixel 10` here,
the path to the relevant files, as shown at the top, is followed.

<img src="images/ANDROID.md/Dolphin.png" width="100%">


# Install

DB48X is not available in the Google Play Store.
The projects github pipeline does build .AAB files.
But these aren not uploaded to the store.

The pipeline also builds .APK files.
The intent (see [PR #1745](https://github.com/c3d/db48x/pull/1745)) is to provide those as a release asset.
Android users can then, on their Android device, download the .APK file and sideload / install it.

<div align="center">
Warning
</div>

A new installation will, at first launch, copy a new version of the above mentioned files over the existing ones.
If you have made changes to any of those files, those changes will be lost.
If you need to keep those changes, you should backup the changed files before you install a new version of the App.
After the installation you can reapply the changes.
Do not replace a new configuraton file with your own older one.
There may be release related changes which should be kept.
Merge your changes into the new file.


# Build

In a future version of this document a description of how to build the App will be given here.