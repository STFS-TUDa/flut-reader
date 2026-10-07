
## Description

## Closed Issues

[//]: # (List any issues that are closed by this Merge Request, typically just one)
[//]: # (Use the keywords from https://docs.gitlab.com/ee/user/project/issues/managing_issues.html#default-closing-pattern)
[//]: # (Example: Closes #123)


## Checklist for Code Reviews
[//]: # (Leave this in if your Merge Request contains code that needs to be reviewed, otherwise remove it)

### Structure
- [ ] Does the Code conform to coding standards?
- [ ] Is the code well-structured, consistent in style, and consistently formatted?

[//]: # (- [ ] Is every procedure needed and called and every statement reachable? -Wunreachable-code)
- [ ] Are leftover stubs and test routines removed from the code?
- [ ] Are calls to external reusable components or library functions used?
- [ ] Are blocks of repeated code condensed into a single procedure?

[//]: # (- [ ] Are symbolics used rather than "magic number" constants or string constants? # Checked by readability-magic-numbers)
- [ ] Are complex modules split into multiple routines?
- [ ] Are different modules separated into different files?
- [ ] Are declaration and definition split in two different files?
- [ ] Are modules named after what they do?
- [ ] Are start and end points of code blocks easily to identify?
- [ ] Does the code use modern language features where it is possible?

### Documentation
- [ ] Is the code clearly and adequately documented with an easy-to-maintain commenting style?
- [ ] Are all comments consistent with the code?

### Variables
- [ ] Are all variables properly defined with meaningful, consistent, and clear names?

[//]: # (- [ ] Do all assigned variables have proper type consistency or casting? # covered by cppcoreguidelines-pro-type-*)
[//]: # (- [ ] Is C++-style casting used instead of C-style casting? # covered by cppcoreguidelines-pro-type-*)
[//]: # (- [ ] Are const_casts avoided? # covered by cppcoreguidelines-pro-type-*)
[//]: # (- [ ] Are there no redundant or unused variables? # -Wunused-variable)
- [ ] Are all variables declared in the innermost scope they are needed in?
- [ ] Are data members of classes private, and if needed using getters and setters?
- [ ] Are constants used rather than #define?

[//]: # (- [ ] Are variables constant where possible? # covered by misc-const-correctness)
- [ ] Are parameters constant where possible?

### Loops and Branches
- [ ] Are all loops, branches, and logic constructs complete, correct, and properly nested?
- [ ] Are the most common cases tested first in IF- -ELSEIF chains?
- [ ] Are all cases covered in an IF- -ELSEIF or CASE block, including ELSE or DEFAULT clauses if needed?

[//]: # (- [ ] Does every case statement have a default? # bugprone-switch-missing-default-case)
- [ ] Are loop termination conditions obvious and invariably achievable?
- [ ] Are indexes or subscripts properly initialized, just prior to the loop?
- [ ] Are there only statements in the loops that need to be?
- [ ] Does the code in the loop avoid manipulating the index variable or using it upon exit from the loop?
- [ ] Are range based loops used wherever possible?