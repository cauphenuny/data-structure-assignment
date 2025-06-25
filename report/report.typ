#import "meta.typ": *
#import "theme.typ": *

#show: doc => conf(
  title: meta.title,
  course: meta.course,
  author: meta.author,
  semester: meta.semester,
  doc,
)

完整代码：#meta.repo

#include "tree.typ"

#include "knights.typ"
