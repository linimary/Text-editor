Program Overview: Text Transformation and Manipulation

This program is designed to read, write, and transform text data. It supports various operations to manipulate input and output data. You can perform the following actions:

- Remove a specific string from the content
- Remove entire lines containing a given string
- Remove a specific character
- Replace one string with another
- Remove punctuation
- Add a new line after each sentence
- Add a new line after each word
- Add new lines to limit each line to a maximum character count (without splitting words if possible)
- Remove new lines
- Lexicographically sort all lines
- Remove duplicate lines
- Output the number of lines in the input data (which becomes the new output content)
- Output the number of characters in the input data (which becomes the new output content)

The program also allows you to group input, perform multiple transformations, and produce output in sequences of tasks. Here are a few example use cases:

- Dictionary Extraction: Read a file, remove punctuation, add new lines after each word, remove duplicate lines, and save the result in a file
- Word Count in Files: Read multiple files, remove punctuation, put each word on a separate line, remove specific words, count their occurrences, and display the counts in the console
- Archiving: Read multiple files and save them in a single output file
- File Splitting: Read one input file and split it into smaller files, each containing a maximum of K characters

Sequences of tasks can be combined, for instance, extracting a dictionary and then splitting it into multiple files.
