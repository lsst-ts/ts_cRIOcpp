# Search for the path containing library's headers
find_path(readline_ROOT_DIR
    NAMES include/readline/readline.h
)

# Search for include directory
find_path(readline_INCLUDE_DIR
    NAMES readline/readline.h
    HINTS ${readline_ROOT_DIR}/include
)

# Search for library
find_library(readline_LIBRARY
    NAMES readline
    HINTS ${readline_ROOT_DIR}/lib
)

# Conditionally set READLINE_FOUND value
if(readline_INCLUDE_DIR AND readline_LIBRARY 
  AND Ncurses_LIBRARY)
  set(READLINE_FOUND TRUE)
else(readline_INCLUDE_DIR AND readline_LIBRARY 
  AND Ncurses_LIBRARY)
  FIND_LIBRARY(readline_LIBRARY NAMES readline)
  include(FindPackageHandleStandardArgs)
  FIND_PACKAGE_HANDLE_STANDARD_ARGS(readline DEFAULT_MSG 
    readline_INCLUDE_DIR readline_LIBRARY )
  MARK_AS_ADVANCED(readline_INCLUDE_DIR readline_LIBRARY)
endif(readline_INCLUDE_DIR AND readline_LIBRARY 
  AND Ncurses_LIBRARY)

# Hide these variables in cmake GUIs
mark_as_advanced(
    readline_ROOT_DIR
    readline_INCLUDE_DIR
    readline_LIBRARY
)
