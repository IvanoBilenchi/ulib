# Project metadata

project = '@PROJECT_NAME@'
copyright = '@ULIB_COPYRIGHT_YEAR@, @ULIB_AUTHOR@'
author = '@ULIB_AUTHOR@'
version = '@PROJECT_VERSION@'
release = '@PROJECT_VERSION@'
git_url = '@ULIB_GIT_URL@'

# Sphinx

primary_domain = 'cpp'
default_role = 'any'
extensions = ['breathe']
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']
rst_prolog = f':github_url: {git_url}'
rst_epilog = f'.. _git_url: {git_url}'

# HTML

html_theme = 'sphinx_rtd_theme'
html_theme_options = { 'logo_only': False }
html_short_title = '{} docs'.format(project)
html_copy_source = False
html_show_sphinx = False
html_use_index = False

# Breathe

breathe_projects = { project: '@DOXYGEN_XML_OUTPUT_DIRECTORY@' }
breathe_default_project = project
breathe_default_members = ('members', 'undocmembers')
