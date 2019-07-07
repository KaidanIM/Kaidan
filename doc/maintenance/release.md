# Releasing New Kaidan Version

* X.Y.Z and N+1 must be adjusted to correspond to the actual version.

1. In repository **https://invent.kde.org/kde/kaidan.git**:
	1. `git checkout master`
	1. `git pull origin master`
	1. [Update translations](translation.md)
	1. Update changelog (*changelog.md*):
		1. Set version vX.Y.Z
		1. Mention important changes and their authors
		1. `git add changelog.md`
		1. `git commit -m "Prepare changelog for vX.Y.Z"`
	1. [Update license information](license.md)
	1. In *CMakeLists.txt*:
		1. Change `set(VERSION_STRING "X.Y.Z-dev")` to `set(VERSION_STRING "X.Y.Z")`
		1. Change `set(DEVELOPMENT_BUILD TRUE)` to `set(DEVELOPMENT_BUILD FALSE)`
		1. Change `set(ANDROID_VERSION_CODE N)` to `set(ANDROID_VERSION_CODE N+1)`
	1. `git add CMakeLists.txt`
	1. Update changelog (*changelog.md*):
		1. Set release date
		1. `git add changelog.md`
	1. `git checkout -b release/vX.Y.Z`
	1. `git commit -m "Release Kaidan vX.Y.Z"`
	1. `git push --set-upstream origin release/vX.Y.Z`
	1. Archive release on *stable* branch:
		1. `git checkout stable`
		1. `git merge master`
		1. `git push origin stable`
		1. `git push -d release/vX.Y.Z`
	1. Add tag on *stable* branch:
		1. `git tag -a vX.Y.Z`
		1. Add changelog part for vX.Y.Z as tag message
		1. `git push origin vX.Y.Z`
		1. Add changelog part for vX.Y.Z as release note on https://invent.kde.org/kde/kaidan/tags
	1. Publish release builds on https://download.kde.org:
		1. Download builds from https://invent.kde.org/kde/kaidan/pipelines?scope=tags&page=1 for tag *vX.Y.Z*
		1. Rename dowloaded builds to *kaidan_X.Y.Z_<architecture>.<type>* (Replace <architecture>.<type> with the corresponding architecture and file type.)
		1. `../releaseme/tarme.rb --version X.Y.Z --origin stable kaidan` (*releasme* is the directory for the repository **https://anongit.kde.org/releaseme.git**.)
		1. `kdecp5 kaidan-X.Y.Z.* ftp://upload.kde.org/incoming`:
			1. Create admin ticket as described in output with additional lines for all uploaded files and hash values by SHA-256 and SHA-1
1. In repository **https://invent.kde.org/websites/kaidan-im.git**:
	1. `git checkout master`
	1. `git pull origin master`
	1. Update screenshot (which will update the screenshot on the website and in the README):
		1. Create one screenshot for desktop and one for mobile version
		1. Put those two screenshots into one image like *images/screenshot.png* and overwrite that by that by the new one
		1. `optipng -o7 images/screenshot.png`
		1. `advpng -z4 images/screenshot.png`
		1. `git add images/screenshot.png`
		1. `git commit -m "Update screenshot"`
		1. `git push origin master`
	1. Update version and date in the following lines of *index.html*:
		* `<h3>Current version: X.Y.Z</h3>`
		* `<h4>(released MONTH DAY, YEAR)</h4>`
	1. Publish blog post:
		1. Create *_posts/YEAR-MONTH-DAY-kaidan-X.Y.Z.md*
		1. Add header as in other posts of that directory
		1. Add changelog part for vX.Y.Z
1. On Mastodon via **https://fosstodon.org/@kaidan**:
	1. Create a toot like https://fosstodon.org/@kaidan/101929341888805925

# Starting New Development Kaidan Version

* X.Y.Z, X.Y+1 and N+1 must be adjusted to correspond to the actual version.

1. `git checkout master`
1. `git pull origin master`
1. In *CMakeLists.txt*:
	1. Change `set(VERSION_STRING "X.Y.Z")` to `set(VERSION_STRING "X.Y+1.0-dev")`
	1. Change `set(DEVELOPMENT_BUILD FALSE)` to `set(DEVELOPMENT_BUILD TRUE)`
	1. Change `set(ANDROID_VERSION_CODE N)` to `set(ANDROID_VERSION_CODE N+1)`
1. `git add CMakeLists.txt`
1. `git commit -m "Continue with Kaidan vX.Y+1.0-dev"`
1. `git push origin master`
