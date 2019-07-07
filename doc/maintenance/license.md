# Updating License Information

These steps should only be followed while [releasing a new Kaidan version](release.md).

1. `git checkout master`
1. `git pull origin master`
1. `utils/generate-license.py > LICENSE` (On Debian `sudo apt install python3-git` may be needed.)
1. Add possible aliases:
	1. `git diff`
	1. If there are contributers with multiple email addresses:
		1. Adjust *utils/generate-license.py*:
			1. Add a line like the following for each alias in REPLACE_USER_IDS = [...]:
			`("Alias <alias@example.org>", "FirstName LastName <user@example.com>"),`
		1. `git add utils/generate-license.py`
		1. `utils/generate-license.py > LICENSE`
1. `git add LICENSE`
