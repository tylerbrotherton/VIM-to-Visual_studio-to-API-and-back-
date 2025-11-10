" --- Configuration (Adjust these as needed) ---

" 1. Path to your compiled C executable
let g:VIM_binary_path = '~/VimHleper.exe'

" 2. The Gemini API endpoint URL, for demo purposes
" Use the model you compiled your C program for, e.g., gemini-2.5-flash
let g:API_endpoint_url = 'https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent'

let g:API_key_env= 'API_KEY'  "This pulls from the VImBigHelper

" This function captures the current visual selection or the current line as the prompt.
function! API_Call(prompt, ...) abort
    if a:0 >=1
        let l:endpoint= a:1
    else 
        let l:endpoint=g.API_endpoint_url
    endif

    " Shell-escape the arguments to handle spaces and special characters
    let l:endpoint_esc = shellescape(g:API_endpoint_url)
    let l:prompt_esc = shellescape(a:prompt)
    let l:binary_esc = shellescape(g:VIM_binary_path)
    
    " Construct the full shell command: <binary> <endpoint> <prompt>
    let l:cmd = l:binary_esc . ' ' . l:endpoint_esc . ' ' . l:prompt_esc
    
    " Use the system() function to execute the command and capture its stdout
    let l:result = system(l:cmd)
    
    " The C program uses std::endl, which includes a newline.
    " system() adds an extra newline, so we strip the last one.
    if len(l:result) > 0 && l:result[-1] == "\n"
        let l:result = l:result[:-2]
    endif
    
    " Check for common error messages from your C code (which uses std::cerr)
    if l:result =~# 'Error:'
        echohl ErrorMsg
        echom "API Error: " . l:result
        echohl None
        return
    endif
    
    return l:result
endfunction

function! APIComplete(A, L, P) abort
    return [g:API_endpoint_url]
endfunction


xnoremap <silent> <leader>ga :<C-u>call s:VisualAPIReplace()<CR>


function! s:VisualAPIReplace() range abort
    " Grab the selected lines as a single prompt string
    let l:prompt = join(getline(a:firstline, a:lastline), "\n")
    let l:output = API_Call(l:prompt)

    if empty(l:output)
        return
    endif

    " Delete the original range and insert the result
    execute a:firstline . ',' . a:lastline . 'delete _'
    call append(a:firstline - 1, split(l:output, "\n"))
endfunction


" 1. Command for Normal/Visual mode (replaces selected text or inserts at cursor)
" The -range=% means it can operate on the whole file, a range of lines, or a visual selection.
" -nargs=1 means it takes one argument (the prompt, unless text is selected).
command! -nargs=+ -complete= customlist, APIComplete, APIAsk call s:APIask(<f-args>)

function! s:APIAsk(...) abort
    if a:0 ==0 
        echohl ErrorMsg
        echom 'Usage: APIAsk [endpoint] <prompt>'
        echohl None
        return
    endif

    if a:1 =~# '^https\?://'
        let l:endpoint= a:1
        let l:prompt = join(a:000[1:], ' ' )
    else
        let l:endpoint=''
        let l:prompt = join(a:100, ' ')
    endif

    let l:output=API_Call(l:prompt, l:endpoint)

    if empty(l:output)
        return
    endif

    call append(line('.'), split(l:output, "\n"))
endfunction



" Helper function for the command
function! s:APIexecute(line1, line2, initial_prompt) abort
    " 1. Determine the prompt
    if a:line1 == a:line2
        " No range or single line: use the command argument as the prompt.
        let l:prompt = a:initial_prompt
        let l:insert_mode = 1 " Will insert the output at the cursor
    else
        " Range selected (Visual Mode or line range): use the selected text as the prompt.
        " NOTE: This is complex. The standard filter command is easier for replacing a range.
        " For simplicity, we'll use the prompt from the command line for all.
        " A common pattern is to use an empty command to signal using the range:
        if empty(a:initial_prompt)
            " Get the selected text as the prompt
            let l:prompt = join(getline(a:line1, a:line2), "\n")
            let l:insert_mode = 0 " Will replace the selected range
        else
            " If a range AND an argument is given, prioritize the argument and insert it.
            let l:prompt = a:initial_prompt
            let l:insert_mode = 1
        endif
    endif

    " 2. Get the result
    let l:output = API_Call(l:prompt)

    if empty(l:output)
        " Handle empty output or errors that were already reported
        return
    endif
    
    " 3. Insert or Replace the text
    if l:insert_mode == 1
        " Insert at the cursor position (appends a new line after the text)
        call append('.', l:output) 
    else
        " Replace the selected range (a:line1 to a:line2)
        " First delete the range, then insert the output
        execute a:line1 . ',' . a:line2 . 'd'
        call append(a:line1 - 1, split(l:output, "\n"))
    endif
endfunction

" 2. A more direct, simple-to-use Normal mode mapping
      nnoremap <leader>gm :call append('.', API_Call(input("API Prompt: ")) . "\n")<CR>

nnoremap <silent> <leader>gm :call s:PromptAndAppend()<CR>

function! s:PromptAndAppend() abort
    let l:prompt = input('API Prompt: ')
    if empty(l:prompt)
        return
    endif
    let l:output = API_Call(l:prompt)
    if empty(l:output)
        return
    endif
    call append(line('.'), split(l:output, "\n"))
endfunction

