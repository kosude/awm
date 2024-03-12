import os


# -- Project information -----------------------------------------------------

project = "Awm Manual"
copyright = "2024 Jack Bennett"
author = "Jack Bennett"

release = os.getenv("CONFPY_VERSION")
version = release


# -- General configuration ---------------------------------------------------

extension = [
    "sphinx_rtd_theme",
    "sphinx_design"
]

templates_path = ["templates"]
exclude_patterns = ["_build", "Thumbs.db", ".DS_Store"]

master_doc = "index"


# -- Options for HTML output -------------------------------------------------

html_static_path = ["static"]

html_theme = "sphinx_rtd_theme"
html_theme_options = {
    "collapse_navigation": True,
    "sticky_navigation": True,
    "navigation_depth": 4,
    "titles_only": False,
    "display_version": True,
    "logo_only": False,
    "prev_next_buttons_location": None,
    "style_external_links": True
}

html_title = "Awm Manual"
html_logo = "static/img/logo.svg"

html_css_files = [
    "css/link-colour.css",
    "css/sidebar.css"
]

html_show_sphinx = False
