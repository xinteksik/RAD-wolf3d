# Publishing this folder

Publish this folder's contents as the repository root. The parent development
workspace, SD backup and validation build are private working material and must
not be uploaded. A short GitHub repository description is in DESCRIPTION.md.

```sh
python3 tools/audit_release.py
git init
git add .
git diff --cached --stat
git commit -m "Add RAD-Wolf source release"
```

Create an empty GitHub repository and follow GitHub's instructions to add its
remote and push. This preparation does not create a remote, publish or send
anything. Review the staged list: it must contain no game files, saves, builds,
upstream checkouts, firmware binaries, personal paths or toolchains.

Keep LICENSE, NOTICE.md, licenses/, third_party/ and original source notices.
Do not advertise the commercial WL6 data as open source. The purchase link and
hash manifest identify compatible data without redistributing it.

A compiled binary release is a separate licensing/source-delivery task; the
source-only folder's upstream links do not automatically satisfy that release.
Read LICENSING.md before attaching any kernel or SD image to GitHub Releases.
