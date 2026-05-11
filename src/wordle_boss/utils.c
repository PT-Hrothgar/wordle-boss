// Fairly low-level C utilities
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdbool.h>
#include <string.h>

#include "utils.h"

/*
  Recursively free a linked list of `word_node`s. The provided argument may be NULL.
*/
void free_word_node_list(word_node *list)
{
    if (list != NULL)
    {
        word_node *next = list->next;
        free(list);
        free_word_node_list(next);
    }
}

/*
  Recursively free a linked list of `guess_node`s. The provided argument may be NULL.
*/
void free_guess_node_list(guess_node *list)
{
    if (list != NULL)
    {
        guess_node *next = list->next;
        free(list);
        free_guess_node_list(next);
    }
}

/*
  Recursively free a linked list of `feedback_node`s. The provided argument may be NULL.
*/
void free_feedback_node_list(feedback_node *list)
{
    if (list != NULL)
    {
        feedback_node *next = list->next;
        free(list);
        free_feedback_node_list(next);
    }
}

/*
  Recursively free a linked list of `int_node`s. The provided argument may be NULL.
*/
void free_int_node_list(int_node *list)
{
    if (list != NULL)
    {
        int_node *next = list->next;
        free(list);
        free_int_node_list(next);
    }
}

/* Return non-zero if the given word consists of N uppercase letters */
int is_valid_word(const char *word)
{
    if (strlen(word) != N)
    {
        return 0;
    }
    for (int i = 0; i < N; i++)
    {
        if (!isupper(word[i]))
        {
            return 0;
        }
    }
    return 1;
}

/*
  Determine if a Python object is a valid word.
  If so, copy it into the provided buffer and return positive value;
  if the Python object is not a string or is an invalid one, return 0;
  if a different error occurs, return negative value.
*/
int python_word_to_string(PyObject *word, char *res)
{
    if (!PyUnicode_Check(word))
    {
        return 0;
    }
    const char *val = PyUnicode_AsUTF8(word);
    if (val == NULL)
    {
        return -1;
    }
    if (!is_valid_word(val))
    {
        return 0;
    }
    strcpy(res, val);
    return 1;
}

/*
  Determine if a Python object is a valid N-element sequence of feedback.
  If so, copy it into the provided buffer and return positive value;
  if the Python object is not a sequence or is an invalid one, return 0;
  if a different error occurs, return negative value and set an error.
*/
int python_feedback_to_arr(PyObject *feedback, char *res)
{
    if (!PySequence_Check(feedback))
    {
        return 0;
    }
    Py_ssize_t len;
    PyObject *obj;
    long val;

    if ((len = PySequence_Length(feedback)) != N)
    {
        return 0;
    }
    for (Py_ssize_t i = 0; i < len; i++)
    {
        if ((obj = PySequence_GetItem(feedback, i)) == NULL)
        {
            return -1;
        }
        if (!PyLong_Check(obj))
        {
            return 0;
        }
        val = PyLong_AsLong(obj);
        if (PyErr_Occurred() != NULL)
        {
            return -1;
        }
        if (val != FB_ABSENT && val != FB_CORRECT && val != FB_PRESENT)
        {
            return 0;
        }
        res[i] = val;
    }
    return 1;
}

/*
  Copy feedback for the given guess and target into provided buffer,
  which must have room for N bytes.
*/
void get_feedback(const char *target, const char *guess, char *res)
{
    bool used_target_letters[N];

    for (int i = 0; i < N; i++)
    {
        res[i] = FB_ABSENT;
        used_target_letters[i] = false;
    }

    // See which slots in 'res' will be green
    for (int i = 0; i < N; i++)
    {
        if (target[i] == guess[i])
        {
            res[i] = FB_CORRECT;
            // Record having used this character, so that
            // we don't try to turn it yellow later.
            used_target_letters[i] = true;
        }
    }

    // See which slots in 'res' will be yellow
    for (int i = 0; i < N; i++)
    {
        if (!used_target_letters[i])
        {
            // This letter was not matched in the right location;
            // see if it was matched in a wrong location
            for (int j = 0; j < N; j++)
            {
                if (guess[j] == target[i] && res[j] == FB_ABSENT)
                {
                    // The feedback for this letter should be yellow.
                    res[j] = FB_PRESENT;
                    break;
                }
            }
        }
    }
}

/* Python wrapper for get_feedback() */
PyObject *get_python_feedback(PyObject *self, PyObject *args)
{
    char *target, *guess;

    if (!PyArg_ParseTuple(args, "ss", &target, &guess))
    {
        return NULL;
    }

    // Validate given strings
    if (!is_valid_word(target))
    {
        PyErr_SetString(PyExc_ValueError, "target must consist of N uppercase letters");
        return NULL;
    }
    if (!is_valid_word(guess))
    {
        PyErr_SetString(PyExc_ValueError, "guess must consist of N uppercase letters");
        return NULL;
    }

    // Get result
    char res[N];
    get_feedback(target, guess, res);

    // Build Python result
    return fb_to_python_fb(res);
}

/*
  Increment the `possibilities` field of the appropriate node of the given list.
  If no existing node has a `feedback` field equal to the given feedback, add a new node for it.
  The provided list may be NULL. Return a pointer to the beginning of the resulting list,
  or NULL on error. If `free_on_error` is true, recursively free the linked list if an error occurs.
*/
feedback_node *inc_feedback_count(feedback_node *fb_list, const char *new_fb, bool free_on_error)
{
    feedback_node *this_fb = fb_list, *prev_fb = NULL;

    while (this_fb != NULL)
    {
        if (!memcmp(this_fb->feedback, new_fb, N))
        {
            this_fb->possibilities++;
            return fb_list;
        }
        prev_fb = this_fb;
        this_fb = this_fb->next;
    }

    // Create a new node
    feedback_node *new_node = malloc(sizeof(feedback_node));
    if (new_node == NULL)
    {
        if (free_on_error)
        {
            free_feedback_node_list(fb_list);
        }
        return NULL;
    }
    memmove(new_node->feedback, new_fb, N);
    new_node->possibilities = 1;

    if (fb_list == NULL)
    {
        // This is the first node; there is nothing to link it to
        return new_node;
    }

    // At least one node already exists; link it to the new one
    prev_fb->next = new_node;
    // Return a pointer to the beginning of the list
    return fb_list;
}

// Return a score based on how revealing the given feedback appears to be
int fb_score(const char *feedback)
{
    const int green_val = (N + 1) * N / 2 + 1;

    int res = 0;
    for (int i = 0; i < N; i++)
    {
        res += ((N - i) *
                (
                    (feedback[i] == FB_CORRECT) ? green_val :
                    ((feedback[i] == FB_PRESENT) ? 1 : 0)
                ));
    }
    return res;
}

/*
  Return a value less than, equal to, or greater than 0 as `l1` is less than, equal to, or greater
  than `l2`.
*/
char int_node_cmp(int_node *l1, int_node *l2)
{
    for (int_node *n1 = l1, *n2 = l2; 1; n1 = n1->next, n2 = n2->next)
    {
        if (n1 == NULL && n2 == NULL)
        {
            return 0;
        }
        else if (n1 == NULL)
        {
            return 1;
        }
        else if (n2 == NULL)
        {
            return -1;
        }
        if (n1->n < n2->n)
        {
            return -1;
        }
        else if (n1->n > n2->n)
        {
            return 1;
        }
    }
}

/*
  Convert a Python list to a C `word_node` list, returning NULL and setting an exception on error.
*/
word_node *word_list_create(PyObject *python_list)
{
    // Ensure list is not empty
    Py_ssize_t length;
    if (!(length = PyList_Size(python_list)))
    {
        PyErr_SetString(PyExc_ValueError, "word_list must not be empty");
        return NULL;
    }

    word_node *res = NULL, *this_word_node = NULL, *prev_word = NULL;
    char this_word[N + 1];
    int success;

    for (Py_ssize_t i = 0; i < length; i++)
    {
        this_word_node = malloc(sizeof(word_node));

        if (prev_word == NULL)
        {
            // This is the first word of the list
            // Even if malloc() just returned NULL, we must mark
            // self->word_list as being empty for when we free it
            res = this_word_node;
        }
        else
        {
            // Link last iteration's node to the just-created one.
            // Again, even if malloc() returned NULL we do this to close
            // off the existing list for free_word_node_list().
            prev_word->next = this_word_node;
        }
        if (this_word_node == NULL)
        {
            free_word_node_list(res);
            return (word_node *) PyErr_NoMemory();
        }
        this_word_node->next = NULL;

        // Convert the Python `str` to a `char *`
        success = python_word_to_string(PyList_GetItem(python_list, i), this_word);
        // Check the success of that operation
        if (success < 0)
        {
            // Some function returned a NULL pointer
            free_word_node_list(res);
            return (word_node *) PyErr_NoMemory();
        }
        else if (success)
        {
            // Everything went well
            strcpy(this_word_node->word, this_word);
        }
        else
        {
            // The Python object was not a valid string
            PyErr_SetString(
                PyExc_ValueError,
                "All words must be strings consisting of N uppercase letters."
            );
            free_word_node_list(res);
            return NULL;
        }
        prev_word = this_word_node;
    }
    return res;
}

/*
  Convert the given feedback to a Python list of integers. Return NULL and set an error on failure.
*/
PyObject *fb_to_python_fb(const char *fb)
{
    PyObject *res = PyList_New(N);
    if (res == NULL)
    {
        return NULL;
    }

    bool success = true;
    PyObject *this_int;

    for (int i = 0; i < N; i++)
    {
        this_int = PyLong_FromLong(fb[i]);
        if (this_int == NULL)
        {
            success = false;
            break;
        }
        PyList_SET_ITEM(res, i, this_int);
    }
    if (!success)
    {
        // Decrement all reference counts
        for (int i = 0; i < N; i++)
        {
            Py_XDECREF(PyList_GET_ITEM(res, i));
        }
        Py_DECREF(res);
        return NULL;
    }
    return res;
}

/*
  Return non-zero if the given linked list contains the given word.
*/
char word_list_contains(const char *word, word_node *list)
{
    for (word_node *this = list; this != NULL; this = this->next)
    {
        if (!strcmp(this->word, word))
        {
            return 1;
        }
    }
    return 0;
}
