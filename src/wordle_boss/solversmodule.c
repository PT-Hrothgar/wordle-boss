#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <limits.h>
#include <stdbool.h>
#include "utils.h"

#if ((PY_VERSION_HEX >> 16) & ((1 << 8) - 1)) < 13
    // Python version is older than 3.13
#   define PRE_313
#endif

// Low-level struct representation of Python Solver object
typedef struct
{
    // Boilerplate
    PyObject_HEAD
    // struct fields
    word_node *word_list;
    word_node *guess_list;
    guess_node *guesses;
} SolverObject;

// Deallocate a Solver
static void Solver_dealloc(PyObject *ob)
{
    SolverObject *self = (SolverObject *) ob;
    if (self != NULL)
    {
        free_word_node_list(self->word_list);
        free_word_node_list(self->guess_list);
        free_guess_node_list(self->guesses);
    }
    Py_TYPE(self)->tp_free(self);
}

// Create a new Solver
static PyObject *Solver_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
    SolverObject *self = (SolverObject *) type->tp_alloc(type, 0);
    if (self != NULL)
    {
        self->guesses = NULL;
        self->word_list = NULL;
        self->guess_list = NULL;
    }
    return (PyObject *) self;
}

// Initialize an existing Solver with a word list and an optional guess list
static int Solver_init(PyObject *op, PyObject *args, PyObject *kwds)
{
    SolverObject *self = (SolverObject *) op;

    // Parse arguments
    PyObject *python_word_list;
    PyObject *python_guess_list = NULL;
    char *kwlist[] = {"word_list", "guess_list", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!|O!", kwlist, &PyList_Type, &python_word_list,
                                     &PyList_Type, &python_guess_list))
    {
        return -1;
    }

    if (python_guess_list == NULL)
    {
        // Use the word list as the guess list
        python_guess_list = python_word_list;
    }

    // Make ourselves strong references to the Python lists
    // (the parsing function does not do that for us)
    Py_INCREF(python_word_list);
    Py_INCREF(python_guess_list);

    word_node *word_list, *guess_list;

    word_list = word_list_create(python_word_list);
    Py_DECREF(python_word_list);
    if (word_list == NULL)
    {
        Py_DECREF(python_guess_list);
        return -1;
    }

    guess_list = word_list_create(python_guess_list);
    Py_DECREF(python_guess_list);
    if (guess_list == NULL)
    {
        free_word_node_list(word_list);
        return -1;
    }

    free_word_node_list(self->word_list);
    free_word_node_list(self->guess_list);
    self->word_list = word_list;
    self->guess_list = guess_list;
    return 0;
}

// Add a guess with feedback to a Solver object
static PyObject *Solver_add_guess(PyObject *op, PyObject *args)
{
    SolverObject *self = (SolverObject *) op;

    // Parse arguments
    PyObject *python_feedback;
    const char *guess;
    if (!PyArg_ParseTuple(args, "sO", &guess, &python_feedback))
    {
        return NULL;
    }

    // Make ourselves a strong reference to the Python sequence object
    // (the parsing function does not do that for us)
    Py_INCREF(python_feedback);

    // Validate given guess
    if (!is_valid_word(guess))
    {
        Py_DECREF(python_feedback);
        PyErr_SetString(PyExc_ValueError, "Guess must be N-letter string.");
        return NULL;
    }

    // Read provided feedback into an array
    char feedback[N];
    int res = python_feedback_to_arr(python_feedback, feedback);
    Py_DECREF(python_feedback);

    if (!res)
    {
        PyErr_SetString(PyExc_ValueError, "Invalid feedback");
        return NULL;
    }
    else if (res < 0)
    {
        return NULL;
    }

    // Ensure guess has not already been added,
    // and find the node representing the most recently added guess
    guess_node *this_guess = self->guesses;
    guess_node *prev_guess = NULL;
    while (this_guess != NULL)
    {
        if (!strcmp(this_guess->word, guess))
        {
            PyErr_SetString(PyExc_RuntimeError, "Guess already exists");
            return NULL;
        }
        prev_guess = this_guess;
        this_guess = this_guess->next;
    }

    // Create and populate new linked list node
    guess_node *new_guess = malloc(sizeof(guess_node));
    if (new_guess == NULL)
    {
        return PyErr_NoMemory();
    }
    new_guess->next = NULL;
    strcpy(new_guess->word, guess);
    memmove(new_guess->feedback, feedback, N);

    // Add new node
    if (prev_guess == NULL)
    {
        // This is the first guess added
        self->guesses = new_guess;
    }
    else
    {
        // One or more guesses already exist
        prev_guess->next = new_guess;
    }

    Py_RETURN_NONE;
}

// Return a list of the guesses and feedback added so far to a Solver object
static PyObject *Solver_get_guesses(PyObject *op, PyObject *Py_UNUSED(dummy))
{
    SolverObject *self = (SolverObject *) op;

    PyObject *res = PyList_New(0);
    if (res == NULL)
    {
        return NULL;
    }

    PyObject *this_tuple, *this_string, *this_fb;
    guess_node *this_guess = self->guesses;
    bool success = true;
    Py_ssize_t length = 0;

    while (this_guess != NULL)
    {
        this_tuple = PyTuple_New(2);
        if (this_tuple == NULL)
        {
            success = false;
            break;
        }
        this_string = PyUnicode_FromString(this_guess->word);
        if (this_string == NULL)
        {
            Py_DECREF(this_tuple);
            success = false;
            break;
        }
        PyTuple_SET_ITEM(this_tuple, 0, this_string);

        this_fb = fb_to_python_fb(this_guess->feedback);
        if (this_fb == NULL)
        {
            Py_DECREF(this_tuple);
            Py_DECREF(this_string);
            success = false;
            break;
        }

        PyTuple_SET_ITEM(this_tuple, 1, this_fb);

        if (PyList_Append(res, this_tuple) == -1)
        {
            for (int i = 0; i < N; i++)
            {
                Py_DECREF(PyTuple_GET_ITEM(this_fb, i));
            }
            Py_DECREF(this_tuple);
            Py_DECREF(this_string);
            Py_DECREF(this_fb);
            success = false;
            break;
        }
        this_guess = this_guess->next;
        length++;
    }

    if (!success)
    {
        for (int i = 0; i < length; i++)
        {
            this_tuple = PyList_GET_ITEM(res, i);
            this_string = PyTuple_GET_ITEM(this_tuple, 0);
            this_fb = PyTuple_GET_ITEM(this_tuple, 1);
            for (int i = 0; i < N; i++)
            {
                Py_DECREF(PyTuple_GET_ITEM(this_fb, i));
            }
            Py_DECREF(this_tuple);
            Py_DECREF(this_string);
            Py_DECREF(this_fb);
        }
        Py_DECREF(res);
        return NULL;
    }
    return res;
}

// Remove a guess (or all guesses) previously added to a Solver object
static PyObject *Solver_remove_guess(PyObject *op, PyObject *args)
{
    SolverObject *self = (SolverObject *) op;

    const char *word = NULL;
    if (!PyArg_ParseTuple(args, "|s", &word))
    {
        return NULL;
    }

    if (word == NULL)
    {
        // Remove all existing guesses
        free_guess_node_list(self->guesses);
        self->guesses = NULL;

        Py_RETURN_NONE;
    }

    // Remove provided guess
    if (!is_valid_word(word))
    {
        PyErr_SetString(PyExc_ValueError, "Word does not consist of N uppercase letters");
        return NULL;
    }

    guess_node *this_guess = self->guesses;
    guess_node *prev_guess = NULL;
    bool removed_word = false;

    while (this_guess != NULL)
    {
        if (!strcmp(this_guess->word, word))
        {
            if (prev_guess == NULL)
            {
                self->guesses = this_guess->next;
            }
            else
            {
                prev_guess->next = this_guess->next;
            }
            free(this_guess);
            removed_word = true;
            break;
        }
        prev_guess = this_guess;
        this_guess = this_guess->next;
    }

    if (!removed_word)
    {
        PyErr_SetString(PyExc_ValueError, "Guess has not been added to self");
        return NULL;
    }

    Py_RETURN_NONE;
}

// Determine if a word could be the target word of a Solver object
bool Solver_is_word_possible(SolverObject *self, const char *word)
{
    char feedback[N];

    for (guess_node *this_guess = self->guesses; this_guess != NULL; this_guess = this_guess->next)
    {
        get_feedback(word, this_guess->word, feedback);
        if (memcmp(feedback, this_guess->feedback, N))
        {
            return false;
        }
    }
    return true;
}

// Python wrapper for Solver_is_word_possible()
static PyObject *Solver_python_is_word_possible(PyObject *op, PyObject *args)
{
    SolverObject *self = (SolverObject *) op;

    const char *word;
    if (!PyArg_ParseTuple(args, "s", &word))
    {
        return NULL;
    }
    if (!is_valid_word(word))
    {
        PyErr_SetString(PyExc_ValueError, "Invalid word");
        return NULL;
    }

    if (Solver_is_word_possible(self, word))
    {
        Py_INCREF(Py_True);
        return Py_True;
    }
    else
    {
        Py_INCREF(Py_False);
        return Py_False;
    }
}

/*
  Create a linked list of all the possible target words for a Solver object.
  Return NULL if there are no possibilities. On error, return NULL and set an error condition.
  (Use PyErr_Occurred() to disambiguate.)
  Take results from self->guess_list if `take_from_guess_list` is true; otherwise take results
  from self->word_list;
*/
word_node *Solver_get_possibilities(SolverObject *self, bool take_from_guess_list)
{
    // Build a linked list of `word_node`s of all the possible words
    word_node *res = NULL, *list = (take_from_guess_list ? self->guess_list : self->word_list);
    word_node *new_word, *previous_word;

    for (word_node *this_word = list; this_word != NULL; this_word = this_word->next)
    {
        if (Solver_is_word_possible(self, this_word->word))
        {
            new_word = malloc(sizeof(word_node));
            if (new_word == NULL)
            {
                free_word_node_list(res);
                return (word_node *) PyErr_NoMemory();
            }
            new_word->next = NULL;
            strcpy(new_word->word, this_word->word);
            if (res == NULL)
            {
                res = new_word;
            }
            else
            {
                previous_word->next = new_word;
            }
            previous_word = new_word;
        }
    }
    return res;
}

// Python wrapper for Solver_get_possibilities()
static PyObject *Solver_python_get_possibilities(PyObject *op, PyObject *Py_UNUSED(dummy))
{
    SolverObject *self = (SolverObject *) op;
    word_node *words = Solver_get_possibilities(self, false);

    if (words == NULL && PyErr_Occurred() != NULL)
    {
        return NULL;
    }

    // Build a Python list containing all the words
    PyObject *res = PyList_New(0);
    if (res == NULL)
    {
        free_word_node_list(words);
        return NULL;
    }

    PyObject *this_string;
    bool success = true;
    Py_ssize_t length = 0;

    for (word_node *this_word = words; this_word != NULL; this_word = this_word->next)
    {
        this_string = PyUnicode_FromString(this_word->word);
        if (this_string == NULL)
        {
            success = false;
            break;
        }
        if (PyList_Append(res, this_string) == -1)
        {
            Py_DECREF(this_string);
            success = false;
            break;
        }
        length++;
    }
    free_word_node_list(words);

    if (!success)
    {
        // Decrement the reference counts of the Python objects we just created
        for (Py_ssize_t i = 0; i < length; i++)
        {
            Py_DECREF(PyList_GET_ITEM(res, i));
        }
        Py_DECREF(res);
        return NULL;
    }
    return res;
}

/*
  Find the most disappointing possible feedback for the given guess, if the given list contains all
  the possibilities for the target word, and copy the feedback into a provided buffer. Provided list
  must not be NULL. On success, return a list, sorted in descending order, of all the numbers of
  possibilities that would be left by the possible responses to `guess`.
*/
int_node *get_min_feedback(word_node *targets, const char *guess, char *res)
{
    // If there is only one possibility left, save some work
    if (targets->next == NULL)
    {
        int_node *res_list = malloc(sizeof(int_node));
        if (res_list == NULL)
        {
            return NULL;
        }
        get_feedback(targets->word, guess, res);
        res_list->next = NULL;
        res_list->n = 1;
        return res_list;
    }

    // A list of all the possible responses, and the number of possibilities they would leave.
    // We'll actually populate it as we go
    feedback_node *feedbacks = NULL;
    char fb[N];

    for (word_node *this_word = targets; this_word != NULL; this_word = this_word->next)
    {
        get_feedback(this_word->word, guess, fb);

        // Record this word as a possibility left by this feedback
        // inc_feedback_count() adds a new node to the given list if necessary
        if ((feedbacks = inc_feedback_count(feedbacks, fb, true)) == NULL)
        {
            return NULL;
        }
    }

    // Copy the numbers in `feedbacks` into an `int_node` list
    int_node *res_list = NULL;
    int_node *this_int, *prev_int;
    // Number of numbers
    int n = 0;

    for (feedback_node *this_fb = feedbacks; this_fb != NULL; this_fb = this_fb->next)
    {
        this_int = malloc(sizeof(int_node));
        if (this_int == NULL)
        {
            free_int_node_list(res_list);
            free_feedback_node_list(feedbacks);
            return NULL;
        }
        this_int->next = NULL;
        this_int->n = this_fb->possibilities;

        if (res_list == NULL)
        {
            res_list = this_int;
        }
        else
        {
            prev_int->next = this_int;
        }
        prev_int = this_int;
        n++;
    }

    if (res_list->next != NULL)
    {
        // Sort those values in descending order (bubble sort algorithm)
        int m, tmp;
        // n represents the number of switches we will do
        // With x elements in the list, we will initially do x - 1 switches,
        // so start by decrementing n by 1 from the number of elements in the list.
        for (n = n - 1; n > 0; n--)
        {
            m = 0;
            for (int_node *n1 = res_list, *n2 = n1->next; m < n; n1 = n2, n2 = n2->next, m++)
            {
                if (n1->n < n2->n)
                {
                    // Switch two values so that smaller number is on the right/closer to the end of
                    // the list
                    tmp = n2->n;
                    n2->n = n1->n;
                    n1->n = tmp;
                }
            }
        }
    }

    // Get the most disappointing-looking feedback of the tied nodes at the beginning of the list
    int min_score = INT_MAX;
    int this_score;
    char worst_fb[N];

    for (feedback_node *this_fb = feedbacks; this_fb != NULL; this_fb = this_fb->next)
    {
        if (this_fb->possibilities == res_list->n)
        {
            if ((this_score = fb_score(this_fb->feedback)) < min_score)
            {
                min_score = this_score;
                memmove(worst_fb, this_fb->feedback, N);
            }
        }
    }

    memmove(res, worst_fb, N);
    free_feedback_node_list(feedbacks);
    return res_list;
}

// Wrapper adapting get_min_feedback() to a Solver method
static PyObject *Solver_python_get_min_feedback(PyObject *op, PyObject *args)
{
    SolverObject *self = (SolverObject *) op;

    // Parse and validate arguments
    const char *word;
    if (!PyArg_ParseTuple(args, "s", &word))
    {
        return NULL;
    }
    if (!is_valid_word(word))
    {
        PyErr_SetString(PyExc_ValueError, "Invalid word");
        return NULL;
    }

    word_node *word_list = Solver_get_possibilities(self, false);
    if (word_list == NULL)
    {
        if (PyErr_Occurred() == NULL)
        {
            PyErr_SetString(PyExc_RuntimeError, "No possibilities are left for the target word.");
        }
        return NULL;
    }

    // Get the result
    char fb[N];
    int_node *int_list = get_min_feedback(word_list, word, fb);
    free_word_node_list(word_list);

    if (int_list == NULL)
    {
        return PyErr_NoMemory();
    }

    // Create a Python integer
    PyObject *py_int = PyLong_FromLong(int_list->n);
    free_int_node_list(int_list);
    if (py_int == NULL)
    {
        return NULL;
    }

    // Convert the feedback into a Python list
    PyObject *py_fb = fb_to_python_fb(fb);
    if (py_fb == NULL)
    {
        Py_DECREF(py_int);
        return NULL;
    }

    // Put the integer and list into a tuple
    PyObject *res = PyTuple_New(2);
    if (res == NULL)
    {
        Py_DECREF(py_int);
        Py_DECREF(py_fb);
        return NULL;
    }
    PyTuple_SET_ITEM(res, 0, py_fb);
    PyTuple_SET_ITEM(res, 1, py_int);
    return res;
}

/*
  If `absurdle_target` is not NULL, find the best guess in the given list according to the MINIMAX
  algorithm that still leaves `absurdle_target` as a possibility. Otherwise, find the best guess in
  the given list overall.

  Copy the guess into the provided buffer, which must have room for N + 1 bytes.
  Both linked lists must be non-NULL; i.e. they must contain at least one word.
  If `absurdle_target` is non-NULL, `targets` must contain it.
  `guesses` should contain all possible guesses, but if `targets` only contains one word, the "best
  guess" will be that word regardless of what is in `guesses`.

  On success, return 0; if all possible guesses would eliminate the Absurdle target, return -2; if a
  different error occurs, return -1.
*/
int best_guess(word_node *guesses, word_node *targets, const char *absurdle_target, char *res)
{
    // Skip a lot of work if there is only one possible target word
    // (We know targets != NULL)
    if (targets->next == NULL)
    {
        strcpy(res, targets->word);
        return 0;
    }

    int_node *best_int_list = NULL;
    int_node *int_list;
    bool new_best;
    char min_feedback[N], feedback[N], best_word[N + 1], cmp_res, this_word_in_targets;
    char best_word_in_targets = 0;

    for (word_node *guess = guesses; guess != NULL; guess = guess->next)
    {
        // Never guess the Absurdle target, as Absurdle will never give all green feedback if it can
        // help it, so guessing the word you're trying to force to be the target is a bad idea
        if (absurdle_target != NULL)
        {
            if (!strcmp(guess->word, absurdle_target))
            {
                continue;
            }
        }

        int_list = get_min_feedback(targets, guess->word, min_feedback);
        if (int_list == NULL)
        {
            free_int_node_list(best_int_list);
            return -1;
        }

        cmp_res = int_node_cmp(int_list, best_int_list);
        this_word_in_targets = word_list_contains(guess->word, targets);

        if (cmp_res < 0 || (cmp_res == 0 && this_word_in_targets && !best_word_in_targets))
        {
            new_best = true;
            if (absurdle_target != NULL)
            {
                // Make sure the worst feedback (which is the feedback Absurdle would give)
                // would not eliminate this word
                // I.e. make sure these two sets of feedback do not differ
                get_feedback(absurdle_target, guess->word, feedback);
                if (memcmp(feedback, min_feedback, N))
                {
                    // The two sets of feedback do differ
                    new_best = false;
                }
            }
            if (new_best)
            {
                free_int_node_list(best_int_list);
                best_int_list = int_list;
                best_word_in_targets = this_word_in_targets;
                strcpy(best_word, guess->word);
            }
            else
            {
                free_int_node_list(int_list);
            }
        }
    }

    // Ensure we found a better guess during that loop
    if (best_int_list == NULL)
    {
        // All guesses were found to eliminate the Absurdle target
        return -2;
    }

    strcpy(res, best_word);
    free_int_node_list(best_int_list);
    return 0;
}

// Python wrapper adapting best_guess() to a Solver method
static PyObject *Solver_python_best_guess(PyObject *op, PyObject *args, PyObject *kwds)
{
    SolverObject *self = (SolverObject *) op;

    // Parse arguments
    bool hard_mode = false;
    const char *absurdle_target = NULL;
    char *kwlist[] = {"hard_mode", "absurdle_target", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ps", kwlist, &hard_mode, &absurdle_target))
    {
        return NULL;
    }

    word_node *targets = Solver_get_possibilities(self, false);
    if (targets == NULL)
    {
        if (PyErr_Occurred() == NULL)
        {
            PyErr_SetString(PyExc_RuntimeError, "No possibilities are left for the target word");
        }
        return NULL;
    }

    word_node *guesses;
    if (hard_mode)
    {
        // Get all the elements of self->guess_list that are not eliminated
        guesses = Solver_get_possibilities(self, true);
        if (guesses == NULL)
        {
            if (PyErr_Occurred() == NULL)
            {
                PyErr_SetString(PyExc_RuntimeError, "All possible guesses have been eliminated");
            }
            free_word_node_list(targets);
            return NULL;
        }
    }
    else
    {
        guesses = self->guess_list;
    }

    if (absurdle_target != NULL)
    {
        if (!is_valid_word(absurdle_target))
        {
            PyErr_SetString(PyExc_ValueError, "Absurdle target must be valid word");
            free_word_node_list(targets);
            if (hard_mode)
            {
                free_word_node_list(guesses);
            }
            return NULL;
        }
        // Ensure `absurdle_target` is one of the possibilities
        if (!word_list_contains(absurdle_target, targets))
        {
            PyErr_SetString(PyExc_ValueError, "Absurdle target is not one of the possibilities");
            free_word_node_list(targets);
            if (hard_mode)
            {
                free_word_node_list(guesses);
            }
            return NULL;
        }
    }

    char res[N + 1];
    int status;

    // Get result
    if (targets->next == NULL)
    {
        // If there is only one possibility, save some work
        strcpy(res, targets->word);
        status = 0;
    }
    else
    {
        status = best_guess(guesses, targets, absurdle_target, res);
    }

    free_word_node_list(targets);
    if (hard_mode)
    {
        free_word_node_list(guesses);
    }

    if (status == -1)
    {
        return PyErr_NoMemory();
    }
    else if (status == -2)
    {
        PyErr_SetString(PyExc_RuntimeError,
                        "All possible guesses would eliminate the Absurdle target.");
        return NULL;
    }

    return PyUnicode_FromString(res);
}

// Methods of a Solver object
static PyMethodDef Solver_methods[] = {
    {"add_guess", Solver_add_guess, METH_VARARGS, "\
Solver.add_guess(self, guess: str, feedback: Sequence[int], /) -> None\n\n\
Add a guess to self.\
"},
    {"get_guesses", Solver_get_guesses, METH_NOARGS, "\
Solver.get_guesses(self, /) -> list[tuple[str, list[int]]]\n\n\
Return all the guesses added so far to self.\
"},
    {"remove_guess", Solver_remove_guess, METH_VARARGS, "\
Solver.remove_guess(self, guess: Optional[str] = None, /) -> None\n\n\
Remove a guess, or all guesses, previously added to self.\
"},
    {"is_word_possible", Solver_python_is_word_possible, METH_VARARGS, "\
Solver.is_word_possible(self, word: str, /) -> bool\n\n\
Determine if a word could be the target word for self.\
"},
    {"get_possibilities", Solver_python_get_possibilities, METH_NOARGS, "\
Solver.get_possibilities(self, /) -> list[str]\n\n\
Get all the remaining possible target words.\
"},
    {"get_min_feedback", Solver_python_get_min_feedback, METH_VARARGS, "\
Solver.get_min_feedback(self, guess: str, /) -> tuple[list[int], int]\n\n\
Get the most \"disappointing\" possible feedback for the given guess.\
"},
    {"best_guess", (PyCFunction) Solver_python_best_guess, METH_VARARGS | METH_KEYWORDS, "\
Solver.best_guess(self, hard_mode: bool = False, absurdle_target: Optional[str] = None) -> str\n\n\
Get the best possible next guess according to the MINIMAX Algorithm.\n\n\
If `absurdle_target` is not None, return the best guess that still will leave it as a \
possibility. If `hard_mode` is True, return the best guess that has not been eliminated by self's \
previous guesses.\
"},
    {NULL}
};

// Lots of information about Solvers for the interpreter
static PyTypeObject SolverType = {
    // Boilerplate
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    // Actual fields for type
    .tp_name = "wordle_boss.solvers.Solver",
    .tp_doc = PyDoc_STR("\
Solver(self, word_list: list[str], guess_list: Optional[list[str]] = None)\n\n\
Python object type providing much of Wordle Boss's functionality.\
"),
    .tp_basicsize = sizeof(SolverObject),
    .tp_itemsize = 0,
    // Can be used as a base type
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_new = Solver_new,
    .tp_init = Solver_init,
    .tp_dealloc = Solver_dealloc,
    .tp_methods = Solver_methods
};

#ifndef PRE_313

// Add attributes to the Python module after creation
static int solvers_module_exec(PyObject *mod)
{
    // Finalize the Solver type object
    if (PyType_Ready(&SolverType) < 0)
    {
        return -1;
    }
    // Add Solver as an attribute to the module
    if (PyModule_AddObjectRef(mod, "Solver", (PyObject *) &SolverType) < 0)
    {
        return -1;
    }
    // Add N, FB_ABSENT, FB_CORRECT, and FB_PRESENT as attributes to the module
    if (PyModule_AddIntConstant(mod, "N", N) < 0)
    {
        return -1;
    }
    if (PyModule_AddIntConstant(mod, "FB_ABSENT", FB_ABSENT) < 0)
    {
        return -1;
    }
    if (PyModule_AddIntConstant(mod, "FB_CORRECT", FB_CORRECT) < 0)
    {
        return -1;
    }
    if (PyModule_AddIntConstant(mod, "FB_PRESENT", FB_PRESENT) < 0)
    {
        return -1;
    }
    return 0;
}

// Slots of the module
static PyModuleDef_Slot solvers_module_slots[] = {
    // Sets the above function to be called just after the module is created
    {Py_mod_exec, solvers_module_exec},
    // Just use this while using static types
    {Py_mod_multiple_interpreters, Py_MOD_MULTIPLE_INTERPRETERS_NOT_SUPPORTED},
    {0, NULL}
};

#endif

// Methods of the module itself
static PyMethodDef solvers_methods[] = {
    {"get_response", get_python_feedback, METH_VARARGS, "\
get_response(target: str, guess: str, /) -> list[int]\n\n\
Get the Wordle algorithm's response to the given guess.\
"},
    {NULL, NULL, 0, NULL}
};

// Definition of the module
static struct PyModuleDef solvers_module = {
    .m_name = "solvers",
    .m_methods = solvers_methods,
    .m_doc = "The C layer of the wordle_boss package",
#   ifndef PRE_313
    .m_slots = solvers_module_slots,
#   endif
    .m_traverse = NULL,
    .m_clear = NULL,
    .m_free = NULL
};

// Initialize the module
// This function is the only thing in this file directly used by the interpreter.
PyMODINIT_FUNC PyInit_solvers(void)
{
    PyObject *m;

#   ifdef PRE_313
    // Finalize the Solver type object
    if (PyType_Ready(&SolverType) < 0)
    {
        return NULL;
    }

    // Create module
    m = PyModule_Create(&solvers_module);
    if (m == NULL)
    {
        return NULL;
    }

    // Try to add Solver type as attribute of module
    Py_INCREF(&SolverType);
    if (PyModule_AddObject(m, "Solver", (PyObject *) &SolverType) < 0) {
        Py_DECREF(&SolverType);
        Py_DECREF(m);
        return NULL;
    }
    // Add N, FB_ABSENT, FB_CORRECT, and FB_PRESENT as attributes to the module
    if (PyModule_AddIntConstant(m, "N", N) < 0)
    {
        return NULL;
    }
    if (PyModule_AddIntConstant(m, "FB_ABSENT", FB_ABSENT) < 0)
    {
        return NULL;
    }
    if (PyModule_AddIntConstant(m, "FB_CORRECT", FB_CORRECT) < 0)
    {
        return NULL;
    }
    if (PyModule_AddIntConstant(m, "FB_PRESENT", FB_PRESENT) < 0)
    {
        return NULL;
    }

#   else

    // Create module
    m = PyModuleDef_Init(&solvers_module);

#   endif

    return m;
}
