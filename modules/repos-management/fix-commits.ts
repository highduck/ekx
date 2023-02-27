import {executeAsync} from "../cli/utils.js";

// TODO: use provided options
const OLD_EMAIL = "ilyak@playtika.com";
const NEW_NAME = "eliasku";
const NEW_EMAIL = "deadbabe@gmail.com";

const rewriteCommitsAuthor = async (workingDir: string) => {
    const envScript = `'
if [ "$GIT_COMMITTER_EMAIL" = "${OLD_EMAIL}" ]
then
  export GIT_COMMITTER_NAME="${NEW_NAME}"
  export GIT_COMMITTER_EMAIL="${NEW_EMAIL}"
fi
if [ "$GIT_AUTHOR_EMAIL" = "${OLD_EMAIL}" ]
then
  export GIT_AUTHOR_NAME="${NEW_NAME}"
  export GIT_AUTHOR_EMAIL="${NEW_EMAIL}"
fi
'`;
    await executeAsync("git", ["filter-branch", envScript, "--tag-name-filter", "cat", "--", "--branches", "--tags"], {workingDir});
    await executeAsync("git", ["push", "origin", "--force", "--all"], {workingDir});
    await executeAsync("git", ["push", "origin", "--force", "--tags"], {workingDir});
};

// example:
// await rewriteCommitsAuthor("../ekx-demo");