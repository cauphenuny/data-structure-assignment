#import "@preview/tablem:0.2.0": tablem, three-line-table
#import "@preview/theorion:0.3.3": *
// #import "@preview/cuti:0.3.0": show-cn-fakebold
// #show: show-cn-fakebold
#import cosmos.rainbow: *
#show: show-theorion
#set text(font: ("New Computer Modern", "Songti SC"), lang: "zh")
#show emph: text.with(font: ("New Computer Modern", "STKaiti"))

#show ref: r => text(blue, r)

#import "@preview/numbly:0.1.0": numbly

#let meta = (
  title: [数据结构期末汇报],
  subtitle: [实习2.4马踏棋盘，实习6.4平衡二叉树操作的演示],
  author: [袁晨圃，李知谦，邱子陶],
  date: datetime.today(),
  institution: [University of Chinese Academy of Sciences],
)

#import "@preview/touying:0.6.1": *

#let theme = "university"

#show: doc => {
  if theme == "university" {
    import themes.university: *
    show: university-theme.with(aspect-ratio: "16-9", config-info(
      title: meta.title,
      subtitle: meta.subtitle,
      author: meta.author,
      date: meta.date,
      institution: meta.institution,
      logo: none,
    ))
    set heading(numbering: numbly("{1}.", default: "1.1"))
    title-slide()
    doc
  } else if theme == "dewdrop" {
    import themes.dewdrop: *
    show: dewdrop-theme.with(
      aspect-ratio: "16-9",
      footer: self => self.info.title,
      navigation: "mini-slides",
      config-info(
        title: meta.title,
        subtitle: meta.subtitle,
        author: meta.author,
        date: meta.date,
        institution: meta.institution,
      ),
      mini-slides: (height: 2.5em, display-subsection: true, display-section: false, linebreaks: false),
    )
    title-slide()
    outline-slide()
    doc
  } else {
    import themes.university: *
    show: university-theme.with(aspect-ratio: "16-9", config-info(
      title: meta.title,
      subtitle: meta.subtitle,
      author: meta.author,
      date: meta.date,
      institution: meta.institution,
      logo: none,
    ))
    title-slide()
    doc
  }
}

#include "tree.typ"

#include "knights.typ"
