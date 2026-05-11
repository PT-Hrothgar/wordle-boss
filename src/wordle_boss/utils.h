#ifndef UTILS_H
#define UTILS_H

// Length of a word
#define N 5
// Possible feedback values
#define FB_ABSENT 0
#define FB_PRESENT 1
#define FB_CORRECT 2

// Node used to build linked lists of N-letter words
typedef struct word_node
{
    struct word_node *next;
    char word[N + 1];  // An actual null-terminated string
} word_node;

// Node used to build linked lists of possible feedbacks,
// and the number of target words each one would leave
typedef struct feedback_node
{
    struct feedback_node *next;
    char feedback[N];  // Not an actual string, but a sequence of bytes which are each
    // either FB_ABSENT, FB_PRESENT, or FB_CORRECT
    int possibilities;
} feedback_node;

// Node used to build linked lists of the guesses and feedback stored in a Solver object
typedef struct guess_node
{
    struct guess_node *next;
    char word[N + 1];  // An actuall null-terminated string
    char feedback[N];  // See `feedback` field of `feedback_node`
} guess_node;

// Node used to build linked lists of integers
typedef struct int_node
{
     struct int_node *next;
     int n;
} int_node;

/*
  Recursively free a linked list of `word_node`s. The provided argument may be NULL.
*/
void free_word_node_list(word_node *list);
/*
  Recursively free a linked list of `guess_node`s. The provided argument may be NULL.
*/
void free_guess_node_list(guess_node *list);
/*
  Recursively free a linked list of `feedback_node`s. The provided argument may be NULL.
*/
void free_feedback_node_list(feedback_node *list);
/*
  Recursively free a linked list of `int_node`s. The provided argument may be NULL.
*/
void free_int_node_list(int_node *list);
/*
  Copy feedback for the given guess and target into provided buffer,
  which must have room for N bytes.
*/
void get_feedback(const char *target, const char *guess, char *res);
/* Return non-zero if the given word consists of N uppercase letters */
int is_valid_word(const char *word);
/*
  Determine if a Python object is a valid word.
  If so, copy it into the provided buffer and return positive value;
  if the Python object is not a string or is an invalid one, return 0;
  if a different error occurs, return negative value.
*/
int python_word_to_string(PyObject *word, char *res);
/*
  Determine if a Python object is a valid N-element sequence of feedback.
  If so, copy it into the provided buffer and return positive value;
  if the Python object is not a sequence or is an invalid one, return 0;
  if a different error occurs, return negative value and set an error.
*/
int python_feedback_to_arr(PyObject *feedback, char *res);
/* Python wrapper for get_feedback() */
PyObject *get_python_feedback(PyObject *self, PyObject *args);
/*
  Increment the `possibilities` field of the appropriate node of the given list.
  If no existing node has a `feedback` field equal to the given feedback, add a new node for it.
  The provided list may be NULL. Return a pointer to the new list, or NULL on error.
  If `free_on_error` is true, recursively free the linked list if an error occurs.
*/
feedback_node *inc_feedback_count(feedback_node *fb_list, const char *new_fb, bool free_on_error);
// Return a score based on how revealing the given feedback appears to be
int fb_score(const char *feedback);
/*
  Return a value less than, equal to, or greater than 0 as `l1` is less than, equal to, or greater
  than `l2`.
*/
char int_node_cmp(int_node *l1, int_node *l2);
/*
  Convert a Python list to a C `word_node` list, returning NULL and setting an exception on error.
*/
word_node *word_list_create(PyObject *python_list);
/*
  Convert the given feedback to a Python list of integers. Return NULL and set an error on failure.
*/
PyObject *fb_to_python_fb(const char *fb);
/*
  Return non-zero if the given linked list contains the given word.
*/
char word_list_contains(const char *word, word_node *list);

#endif // UTILS_H
