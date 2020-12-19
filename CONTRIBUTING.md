# Contributing

This is a guideline for contributing to Kaidan.
There is also a guide for a [basic setup](https://invent.kde.org/network/kaidan/-/wikis/setup) targeted at contributors which are unfamiliar with KDE Identity, GitLab or Git and want to start contributing quickly.

If you would like to contribute to Kaidan, please create branches on your repository clone and submit merge requests for them.
Please do not create branches on Kaidan's main repository or push your commits directly to its *master* branch.

## Branches

Use the following prefixes for branch names:
* `feature/` for new features (changes with new behavior)
* `refactor/` for changes of the code structure without changing the general behavior
* `fix/` for fixes (changes for intended / correct behavior)
* `design/` for design changes
* `doc/` for documentation

## Merge Requests (MR)

Currently, Linus Jahn (@lnj) and Jonah Brüchert (@jbbgameich) are the maintainers of Kaidan.
They are responsible for accepting MRs.
Nevertheless, everybody is welcome to review MRs and give feedback on them.

Please stick to the following steps for opening, reviewing and accepting MRs.

### For Authors

1. Create a new branch to work on it from the *master* branch.
1. Write short commit messages starting with an upper case letter and the imperative.
1. Split your commits logically.
1. Do not mix unrelated changes in the same MR.
1. Create a MR with the *master* branch as its target.
1. Add `Draft: ` in front of the MR's title as long as you are working on the MR and remove it as soon as it is ready to be reviewed.
1. A maintainer and possibly other reviewers will give you feedback.
1. Improve your MR according to their feedback, push your commits and close open threads via the *Resolve thread* button.
1. If necessary, modify, reorder or squash your commits and force-push (`git push -f`) the result to the MR's branch.
1. If there are no open threads on your MR, a maintainer will merge your commits into the *master* branch.

Please do not merge your commits into the *master* branch by yourself.
If maintainers approved your MR but have not yet merged it, that probably means that they are waiting for the approval of additional maintainers.
Feel free to ask if anything is unclear.

### For Reviewers

1. Provide detailed descriptions of found issues to the author.
1. Try to give the author concrete proposals for improving the code via the *Insert suggestion* button while commenting.
1. If the proposals are too complicated, create and push a commit with your proposal to your own fork of Kaidan and open a MR with the author's MR branch as its target.
1. In case you are a maintainer:
	1. If you think that no additional review is needed, make editorial modifications (such as squashing the commits) and merge the result directly.
	1. If you would like to get (more) feedback from other maintainers, approve the MR using the *Approve* button and mention other maintainers to review the MR.
1. In case you are not a maintainer and you think that the MR is ready to be merged, approve the MR using the *Approve* button.

Reviews have to be done by at least one maintainer not involved as the MR's author or co-author.

## Graphics

The preferred format for graphics in Kaidan is *SVG*.
If SVG is not applicable like for screenshots, the graphic should have the format *PNG*.

### Optimization

In any case, the new or modified graphic must be [optimized](https://invent.kde.org/network/kaidan/-/wikis/optimizing-graphics) before adding it to a commit.

### Copyright

The *LICENSE* file must be updated if there are copyright changes:

1. Add to *utils/generate-license.py* a new `CopyrightTarget` for a new graphic or change an existing one for a modification of an existing graphic.
1. Execute `utils/generate-license.py > LICENSE` for updating the *LICENSE* file.
1. Add those two file changes to the same commit which contains the new or modified graphic.

### Logo

If the logo is changed, it has to be done in a separate commit.
Furthermore, the logo has to be updated on multiple other places:

1. For this GitLab project by creating a [sysadmin request](https://go.kde.org/systickets) and providing a link to a PNG version in its description.
1. In the [repository of Kaidan's website](https://invent.kde.org/websites/kaidan-im) by updating all instances of `favicon*` and `logo*`.
1. In [Kaidan's support chat](xmpp:kaidan@muc.kaidan.im?join) by uploading a new avatar.
