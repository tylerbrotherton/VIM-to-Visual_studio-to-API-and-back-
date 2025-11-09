chmod +x ~/scripts/api_caller.py

---

### 2. The VimL Configuration (.vimrc)

Next, add the following code to your `~/.vimrc` file. This code creates the VimL function and the custom `:CallAPI` command.

I've created a text file with the VimL code for you.


http://googleusercontent.com/immersive_entry_chip/1

### How to Use It

After you've saved the Python script, made it executable, and added the VimL code to your `.vimrc`, restart Vim.

You can now use the new `:CallAPI` command from the Vim command line. It takes two arguments:

1.  The API URL **template**, with `%s` where the parameter should go.
2.  The parameter string you want to pass.

**Example:**

Let's use a public test API, `https://httpbin.org/get`, which just echoes back any parameters it receives.

In Vim, type the following and press Enter:

`:CallAPI "https://httpbin.org/get?query=%s" "hello vim!"`

**What happens:**

1.  Vim calls the `s:CallAPIAndDisplay` function.
2.  The function runs the command: `python3 ~/scripts/api_caller.py "https://httpbin.org/get?query=%s" "hello vim!"`
3.  The Python script URL-encodes `"hello vim!"` to `"hello+vim%21"`.
4.  It fetches the URL `https://httpbin.org/get?query=hello+vim%21`.
5.  Vim captures the JSON response from the API.
6.  A new Vim tab opens, and the JSON output is pasted into the buffer.
