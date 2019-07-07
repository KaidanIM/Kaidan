# Adding Translations Repository

`git remote add weblate https://hosted.weblate.org/git/kaidan/translations/`

# Updating Translations

1. <a name="update-weblate"/> Update translations from Weblate:
	1. On https://hosted.weblate.org/projects/kaidan/translations/#repository with a developer account:
		1. Click *Commit*
		1. Click *Rebase*
	1. `git pull weblate master`
	1. `git push origin master`
	1. If there is a push error related to the policy of KDE which specifies that at least a first and a last name must be set for the author field:
		1. Prepend X for a missing field to the specific commit (possibly by `rebase -i`)
		1. `git push origin master`

1. Update translation sources:
	1. `git checkout master`
	1. `git pull origin master`
	1. `cd build && make -j$(nproc) && cd -`
	1. `git add i18n`
	1. `git commit -m "Update translation sources" --author "Kaidan translations <translations@kaidan.im>"`
	1. `git push origin master`
1. Shortly before the release follow the [first steps](#update-weblate) again.
