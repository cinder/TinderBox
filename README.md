### TinderBox

<p align="center">
  <img src="https://github.com/cinder/TinderBox/blob/master/icons/TinderBox.png" alt="Cinder Logo" width="200" height="auto"/>
</p>

TinderBox is the project creation tool for the C++ creative coding framework [Cinder](https://libcinder.org). It depends on Qt version 5.8.

Binaries of TinderBox are included in the [Cinder repository](https://github.com/cinder/Cinder), so building it from scratch is only necessary if you're working on TinderBox itself.

####Build instructions:####
* Download Qt 5.x from: [http://www.qt.io/download-open-source/](http://www.qt.io/download-open-source/)
* Run the installer. You can skip the account registration if you prefer.
* Mac users can disable everything besides Qt Creator and **clang 64-bit** under Qt 5.8
* Windows users can disable everything besides Qt Creator and **msvc2015 64-bit** under Qt 5.8
* Once installation is complete, open TinderBox.pro from the repo, which should in turn open with Qt Creator.
* The first time you do so, a Configure Project dialog will appear. The defaults are fine - press the **Configure Project** button
* After Qt Creator reads the project, you can build and run it by pressing the green Play button icon in the lower left corner.
