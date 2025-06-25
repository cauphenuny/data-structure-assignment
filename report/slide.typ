#set text(font: ("New Computer Modern", "Songti SC", "SimSun"), lang: "zh")
#show emph: text.with(font: ("New Computer Modern", "STKaiti"))

#import "meta.typ": *
#import "@preview/touying:0.6.1": *
#import "@preview/numbly:0.1.0": *

#show raw.where(block: true): text.with(size: 0.8em)

#show: doc => {
  import themes.university: *
  show: university-theme.with(aspect-ratio: "16-9", config-info(
    title: meta.slide-title,
    subtitle: meta.subtitle,
    author: meta.author,
    date: meta.date,
    institution: meta.institution,
    logo: none,
  ))
  show: text.with(size: 0.75em)
  set heading(numbering: numbly("{1}.", default: "1.1"))
  title-slide()
  doc
  focus-slide[
    Thanks!
  ]
}

#include "knights.typ"

#include "tree.typ"
