# Contributing

This is a guideline for contributing to Kaidan.
There is also a guide for a [basic setup](https://invent.kde.org/network/kaidan/-/wikis/setup) targeted at contributors which are unfamiliar with KDE Identity, GitLab or Git and want to start contributing quickly.

## Branches

Use the following prefixes for branch names:
* `feature/` for new features (changes with new behavior)
* `refactor/` for changes of the code structure without changing the general behavior
* `fix/` for fixes (changes for intended / correct behavior)
* `design/` for design changes
* `doc/` for documentation

## Merge Requests (MR)

Please stick to the following steps for opening and reviewing MRs.

### For Authors

1. Create a new branch to work on it from the *master* branch.
1. Write short commit messages starting with an upper case letter and the imperative.
1. Split your commits logically.
1. Do not mix unrelated changes in the same MR.
1. Create a MR with the *master* branch as its target.
1. Add *WIP: * in front of the MR's title as long as you are working on the MR and remove it as soon as it is ready to be reviewed.
1. If everything is ready to be reviewed, mention at least one person not involved as an author or co-author to review the current state.
1. If necessary, modify, reorder or squash your commits and force-push (`git push -f`) the result to the MR's branch.
1. If there are no open threads on your MR, a reviewer with write access will merge your commits.

### For Reviewers

1. Try to provide a review to the author.
1. Try to give the author concrete proposals for improving the code via the *insert suggestion* feature while commenting.
1. If the proposals are too complicated, create and push a commit with your proposal to your own fork of Kaidan and open a MR with the author's MR branch as its target.

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
