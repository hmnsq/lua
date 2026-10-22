/*
** $Id: llex.c $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#define llex_c
#define LUA_CORE

#அந்தர்பாவ "lprefix.h"


#அந்தர்பாவ <locale.h>
#அந்தர்பாவ <string.h>

#அந்தர்பாவ "lua.h"

#அந்தர்பாவ "lctype.h"
#அந்தர்பாவ "ldebug.h"
#அந்தர்பாவ "ldo.h"
#அந்தர்பாவ "lgc.h"
#அந்தர்பாவ "llex.h"
#அந்தர்பாவ "lobject.h"
#அந்தர்பாவ "lparser.h"
#அந்தர்பாவ "lstate.h"
#அந்தர்பாவ "lstring.h"
#அந்தர்பாவ "ltable.h"
#அந்தர்பாவ "lzio.h"



#define next(ls)	(ls->current = zgetc(ls->z))


/* minimum size for string buffer */
#எனில் !defined(LUA_MINBUFFER)
#define LUA_MINBUFFER   32
#endif


#define currIsNewline(ls)	(ls->current == '\n' || ls->current == '\r')


/* ORDER RESERVED */
நிலையான மாறிலி எழுத்து *மாறிலி luaX_tokens [] = {
    "and", "break", "do", "else", "elseif",
    "end", "false", "for", "function", "global", "goto", "if",
    "in", "local", "nil", "not", "or", "repeat",
    "return", "then", "true", "until", "while",
    "//", "..", "...", "==", ">=", "<=", "~=",
    "<<", ">>", "::", "<eof>",
    "<number>", "<integer>", "<name>", "<string>"
};


#define save_and_next(ls) (save(ls, ls->current), next(ls))


நிலையான l_noret lexerror (LexState *ls, மாறிலி எழுத்து *செய்தி, முழு token);


நிலையான வெறுமை save (LexState *ls, முழு c) {
  Mbuffer *b = ls->buff;
  எனில் (luaZ_bufflen(b) + 1 > luaZ_sizebuffer(b)) {
    size_t newsize = luaZ_sizebuffer(b);  /* get old size */;
    எனில் (newsize >= (MAX_SIZE/3 * 2))  /* larger than MAX_SIZE/1.5 ? */
      lexerror(ls, "lexical element too long", 0);
    newsize += (newsize >> 1);  /* new size is 1.5 times the old one */
    luaZ_resizebuffer(ls->L, b, newsize);
  }
  b->buffer[luaZ_bufflen(b)++] = cast_char(c);
}


வெறுமை luaX_init (lua_State *L) {
  முழு i;
  TString *e = luaS_newliteral(L, LUA_ENV);  /* create env name */
  luaC_fix(L, obj2gco(e));  /* never collect this name */
  க்கு (i=0; i<இலக்கம்_RESERVED; i++) {
    TString *ts = luaS_new(L, luaX_tokens[i]);
    luaC_fix(L, obj2gco(ts));  /* reserved words are never collected */
    ts->extra = cast_byte(i+1);  /* reserved word */
  }
}


மாறிலி எழுத்து *luaX_token2str (LexState *ls, முழு token) {
  எனில் (token < FIRST_RESERVED) {  /* single-byte symbols? */
    எனில் (lisprint(token))
      திரும்பு luaO_pushfstring(ls->L, "'%c'", token);
    இல்லையெனில்  /* control character */
      திரும்பு luaO_pushfstring(ls->L, "'<\\%d>'", token);
  }
  இல்லையெனில் {
    மாறிலி எழுத்து *s = luaX_tokens[token - FIRST_RESERVED];
    எனில் (token < TK_EOS)  /* fixed format (symbols and reserved words)? */
      திரும்பு luaO_pushfstring(ls->L, "'%s'", s);
    இல்லையெனில்  /* names, strings, and numerals */
      திரும்பு s;
  }
}


நிலையான மாறிலி எழுத்து *txtToken (LexState *ls, முழு token) {
  தேர்வு (token) {
    நிலை TK_NAME: நிலை TK_STRING:
    நிலை TK_FLT: நிலை TK_INT:
      save(ls, '\0');
      திரும்பு luaO_pushfstring(ls->L, "'%s'", luaZ_buffer(ls->buff));
    இயல்பு:
      திரும்பு luaX_token2str(ls, token);
  }
}


நிலையான l_noret lexerror (LexState *ls, மாறிலி எழுத்து *செய்தி, முழு token) {
  செய்தி = luaG_addinfo(ls->L, செய்தி, ls->source, ls->linenumber);
  எனில் (token)
    luaO_pushfstring(ls->L, "%s near %s", செய்தி, txtToken(ls, token));
  luaD_throw(ls->L, LUA_ERRSYNTAX);
}


l_noret luaX_syntaxerror (LexState *ls, மாறிலி எழுத்து *செய்தி) {
  lexerror(ls, செய்தி, ls->t.token);
}


/*
** Anchors a string in scanner's table so that it will not be collected
** until the end of the compilation; by that time it should be anchored
** somewhere. It also internalizes long strings, ensuring there is only
** one copy of each unique string.
*/
நிலையான TString *anchorstr (LexState *ls, TString *ts) {
  lua_State *L = ls->L;
  TValue oldts;
  முழு tag = luaH_getstr(ls->h, ts, &oldts);
  எனில் (!tagisempty(tag))  /* string already present? */
    திரும்பு tsvalue(&oldts);  /* use stored value */
  இல்லையெனில் {  /* create a new entry */
    TValue *stv = s2v(L->உச்சி.p++);  /* reserve stack space for string */
    setsvalue(L, stv, ts);  /* push (anchor) the string on the stack */
    luaH_set(L, ls->h, stv, stv);  /* t[string] = string */
    /* table is not a metatable, so it does not need to invalidate cache */
    luaC_checkGC(L);
    L->உச்சி.p--;  /* remove string from stack */
    திரும்பு ts;
  }
}


/*
** Creates a new string and anchors it in scanner's table.
*/
TString *luaX_newstring (LexState *ls, மாறிலி எழுத்து *str, size_t l) {
  திரும்பு anchorstr(ls, luaS_newlstr(ls->L, str, l));
}


/*
** increment line number and skips newline sequence (any of
** \n, \r, \n\r, or \r\n)
*/
நிலையான வெறுமை inclinenumber (LexState *ls) {
  முழு old = ls->current;
  lua_assert(currIsNewline(ls));
  next(ls);  /* skip '\n' or '\r' */
  எனில் (currIsNewline(ls) && ls->current != old)
    next(ls);  /* skip '\n\r' or '\r\n' */
  எனில் (++ls->linenumber >= INT_MAX)
    lexerror(ls, "chunk has too many lines", 0);
}


வெறுமை luaX_setinput (lua_State *L, LexState *ls, ZIO *z, TString *source,
                    முழு firstchar) {
  ls->t.token = 0;
  ls->L = L;
  ls->current = firstchar;
  ls->lookahead.token = TK_EOS;  /* no look-ahead token */
  ls->z = z;
  ls->fs = வெற்று;
  ls->linenumber = 1;
  ls->lastline = 1;
  ls->source = source;
  /* all three strings here ("_ENV", "break", "global") were fixed,
     so they cannot be collected */
  ls->envn = luaS_newliteral(L, LUA_ENV);  /* get env string */
  ls->brkn = luaS_newliteral(L, "break");  /* get "break" string */
#எனில் LUA_COMPAT_GLOBAL
  /* compatibility mode: "global" is not a reserved word */
  ls->glbn = luaS_newliteral(L, "global");  /* get "global" string */
  ls->glbn->extra = 0;  /* mark it as not reserved */
#endif
  luaZ_resizebuffer(ls->L, ls->buff, LUA_MINBUFFER);  /* initialize buffer */
}



/*
** =======================================================
** LEXICAL ANALYZER
** =======================================================
*/


நிலையான முழு check_next1 (LexState *ls, முழு c) {
  எனில் (ls->current == c) {
    next(ls);
    திரும்பு 1;
  }
  இல்லையெனில் திரும்பு 0;
}


/*
** Check whether current char is in set 'set' (with two chars) and
** saves it
*/
நிலையான முழு check_next2 (LexState *ls, மாறிலி எழுத்து *set) {
  lua_assert(set[2] == '\0');
  எனில் (ls->current == set[0] || ls->current == set[1]) {
    save_and_next(ls);
    திரும்பு 1;
  }
  இல்லையெனில் திரும்பு 0;
}


/* LUA_NUMBER */
/*
** This function is quite liberal in what it accepts, as 'luaO_str2num'
** will reject ill-formed numerals. Roughly, it accepts the following
** pattern:
**
**   %d(%x|%.|([Ee][+-]?))* | 0[Xx](%x|%.|([Pp][+-]?))*
**
** The only tricky part is to accept [+-] only after a valid exponent
** mark, to avoid reading '3-4' or '0xe+1' as a single number.
**
** The caller might have already read an initial dot.
*/
நிலையான முழு read_numeral (LexState *ls, SemInfo *seminfo) {
  TValue obj;
  மாறிலி எழுத்து *expo = "Ee";
  முழு first = ls->current;
  lua_assert(lisdigit(ls->current));
  save_and_next(ls);
  எனில் (first == '0' && check_next2(ls, "xX"))  /* hexadecimal? */
    expo = "Pp";
  க்கு (;;) {
    எனில் (check_next2(ls, expo))  /* exponent mark? */
      check_next2(ls, "-+");  /* optional exponent sign */
    இல்லையெனில் எனில் (lisxdigit(ls->current) || ls->current == '.')  /* '%x|%.' */
      save_and_next(ls);
    இல்லையெனில் நிறுத்து;
  }
  எனில் (lislalpha(ls->current))  /* is numeral touching a letter? */
    save_and_next(ls);  /* force an error */
  save(ls, '\0');
  எனில் (luaO_str2num(luaZ_buffer(ls->buff), &obj) == 0)  /* format error? */
    lexerror(ls, "malformed number", TK_FLT);
  எனில் (ttisinteger(&obj)) {
    seminfo->i = ivalue(&obj);
    திரும்பு TK_INT;
  }
  இல்லையெனில் {
    lua_assert(ttisfloat(&obj));
    seminfo->r = fltvalue(&obj);
    திரும்பு TK_FLT;
  }
}


/*
** read a sequence '[=*[' or ']=*]', leaving the last bracket. If
** sequence is well formed, return its number of '='s + 2; otherwise,
** return 1 if it is a single bracket (no '='s and no 2nd bracket);
** otherwise (an unfinished '[==...') return 0.
*/
நிலையான size_t skip_sep (LexState *ls) {
  size_t மொத்தம் = 0;
  முழு s = ls->current;
  lua_assert(s == '[' || s == ']');
  save_and_next(ls);
  போது (ls->current == '=') {
    save_and_next(ls);
    மொத்தம்++;
  }
  திரும்பு (ls->current == s) ? மொத்தம் + 2
         : (மொத்தம் == 0) ? 1
         : 0;
}


நிலையான வெறுமை read_long_string (LexState *ls, SemInfo *seminfo, size_t sep) {
  முழு வரி = ls->linenumber;  /* initial line (for error message) */
  save_and_next(ls);  /* skip 2nd '[' */
  எனில் (currIsNewline(ls))  /* string starts with a newline? */
    inclinenumber(ls);  /* skip it */
  க்கு (;;) {
    தேர்வு (ls->current) {
      நிலை EOZ: {  /* error */
        மாறிலி எழுத்து *what = (seminfo ? "string" : "comment");
        மாறிலி எழுத்து *செய்தி = luaO_pushfstring(ls->L,
                     "unfinished long %s (starting at line %d)", what, வரி);
        lexerror(ls, செய்தி, TK_EOS);
        நிறுத்து;  /* to avoid warnings */
      }
      நிலை ']': {
        எனில் (skip_sep(ls) == sep) {
          save_and_next(ls);  /* skip 2nd ']' */
          செல் endloop;
        }
        நிறுத்து;
      }
      நிலை '\n': நிலை '\r': {
        save(ls, '\n');
        inclinenumber(ls);
        எனில் (!seminfo) luaZ_resetbuffer(ls->buff);  /* avoid wasting space */
        நிறுத்து;
      }
      இயல்பு: {
        எனில் (seminfo) save_and_next(ls);
        இல்லையெனில் next(ls);
      }
    }
  } endloop:
  எனில் (seminfo)
    seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + sep,
                                     luaZ_bufflen(ls->buff) - 2 * sep);
}


நிலையான வெறுமை esccheck (LexState *ls, முழு c, மாறிலி எழுத்து *செய்தி) {
  எனில் (!c) {
    எனில் (ls->current != EOZ)
      save_and_next(ls);  /* add current to buffer for error message */
    lexerror(ls, செய்தி, TK_STRING);
  }
}


நிலையான முழு gethexa (LexState *ls) {
  save_and_next(ls);
  esccheck (ls, lisxdigit(ls->current), "hexadecimal digit expected");
  திரும்பு luaO_hexavalue(ls->current);
}


நிலையான முழு readhexaesc (LexState *ls) {
  முழு r = gethexa(ls);
  r = (r << 4) + gethexa(ls);
  luaZ_buffremove(ls->buff, 2);  /* remove saved chars from buffer */
  திரும்பு r;
}


/*
** When reading a UTF-8 escape sequence, save everything to the buffer
** for error reporting in case of errors; 'i' counts the number of
** saved characters, so that they can be removed if case of success.
*/
நிலையான l_uint32 readutf8esc (LexState *ls) {
  l_uint32 r;
  முழு i = 4;  /* number of chars to be removed: start with #"\u{X" */
  save_and_next(ls);  /* skip 'u' */
  esccheck(ls, ls->current == '{', "missing '{'");
  r = cast_uint(gethexa(ls));  /* must have at least one digit */
  போது (cast_void(save_and_next(ls)), lisxdigit(ls->current)) {
    i++;
    esccheck(ls, r <= (0x7FFFFFFFu >> 4), "UTF-8 value too large");
    r = (r << 4) + luaO_hexavalue(ls->current);
  }
  esccheck(ls, ls->current == '}', "missing '}'");
  next(ls);  /* skip '}' */
  luaZ_buffremove(ls->buff, i);  /* remove saved chars from buffer */
  திரும்பு r;
}


நிலையான வெறுமை utf8esc (LexState *ls) {
  எழுத்து buff[UTF8BUFFSZ];
  முழு n = luaO_utf8esc(buff, readutf8esc(ls));
  க்கு (; n > 0; n--)  /* add 'buff' to string */
    save(ls, buff[UTF8BUFFSZ - n]);
}


நிலையான முழு readdecesc (LexState *ls) {
  முழு i;
  முழு r = 0;  /* result accumulator */
  க்கு (i = 0; i < 3 && lisdigit(ls->current); i++) {  /* read up to 3 digits */
    r = 10*r + ls->current - '0';
    save_and_next(ls);
  }
  esccheck(ls, r <= UCHAR_MAX, "decimal escape too large");
  luaZ_buffremove(ls->buff, i);  /* remove read digits from buffer */
  திரும்பு r;
}


நிலையான வெறுமை read_string (LexState *ls, முழு del, SemInfo *seminfo) {
  save_and_next(ls);  /* keep delimiter (for error messages) */
  போது (ls->current != del) {
    தேர்வு (ls->current) {
      நிலை EOZ:
        lexerror(ls, "unfinished string", TK_EOS);
        நிறுத்து;  /* to avoid warnings */
      நிலை '\n':
      நிலை '\r':
        lexerror(ls, "unfinished string", TK_STRING);
        நிறுத்து;  /* to avoid warnings */
      நிலை '\\': {  /* escape sequences */
        முழு c;  /* final character to be saved */
        save_and_next(ls);  /* keep '\\' for error messages */
        தேர்வு (ls->current) {
          நிலை 'a': c = '\a'; செல் read_save;
          நிலை 'b': c = '\b'; செல் read_save;
          நிலை 'f': c = '\f'; செல் read_save;
          நிலை 'n': c = '\n'; செல் read_save;
          நிலை 'r': c = '\r'; செல் read_save;
          நிலை 't': c = '\t'; செல் read_save;
          நிலை 'v': c = '\v'; செல் read_save;
          நிலை 'x': c = readhexaesc(ls); செல் read_save;
          நிலை 'u': utf8esc(ls);  செல் no_save;
          நிலை '\n': நிலை '\r':
            inclinenumber(ls); c = '\n'; செல் only_save;
          நிலை '\\': நிலை '\"': நிலை '\'':
            c = ls->current; செல் read_save;
          நிலை EOZ: செல் no_save;  /* will raise an error next loop */
          நிலை 'z': {  /* zap following span of spaces */
            luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
            next(ls);  /* skip the 'z' */
            போது (lisspace(ls->current)) {
              எனில் (currIsNewline(ls)) inclinenumber(ls);
              இல்லையெனில் next(ls);
            }
            செல் no_save;
          }
          இயல்பு: {
            esccheck(ls, lisdigit(ls->current), "invalid escape sequence");
            c = readdecesc(ls);  /* digital escape '\ddd' */
            செல் only_save;
          }
        }
       read_save:
         next(ls);
         /* go through */
       only_save:
         luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
         save(ls, c);
         /* go through */
       no_save: நிறுத்து;
      }
      இயல்பு:
        save_and_next(ls);
    }
  }
  save_and_next(ls);  /* skip delimiter */
  seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + 1,
                                   luaZ_bufflen(ls->buff) - 2);
}


நிலையான முழு llex (LexState *ls, SemInfo *seminfo) {
  luaZ_resetbuffer(ls->buff);
  க்கு (;;) {
    தேர்வு (ls->current) {
      நிலை '\n': நிலை '\r': {  /* line breaks */
        inclinenumber(ls);
        நிறுத்து;
      }
      நிலை ' ': நிலை '\f': நிலை '\t': நிலை '\v': {  /* spaces */
        next(ls);
        நிறுத்து;
      }
      நிலை '-': {  /* '-' or '--' (comment) */
        next(ls);
        எனில் (ls->current != '-') திரும்பு '-';
        /* else is a comment */
        next(ls);
        எனில் (ls->current == '[') {  /* long comment? */
          size_t sep = skip_sep(ls);
          luaZ_resetbuffer(ls->buff);  /* 'skip_sep' may dirty the buffer */
          எனில் (sep >= 2) {
            read_long_string(ls, வெற்று, sep);  /* skip long comment */
            luaZ_resetbuffer(ls->buff);  /* previous call may dirty the buff. */
            நிறுத்து;
          }
        }
        /* else short comment */
        போது (!currIsNewline(ls) && ls->current != EOZ)
          next(ls);  /* skip until end of line (or end of file) */
        நிறுத்து;
      }
      நிலை '[': {  /* long string or simply '[' */
        size_t sep = skip_sep(ls);
        எனில் (sep >= 2) {
          read_long_string(ls, seminfo, sep);
          திரும்பு TK_STRING;
        }
        இல்லையெனில் எனில் (sep == 0)  /* '[=...' missing second bracket? */
          lexerror(ls, "invalid long string delimiter", TK_STRING);
        திரும்பு '[';
      }
      நிலை '=': {
        next(ls);
        எனில் (check_next1(ls, '=')) திரும்பு TK_EQ;  /* '==' */
        இல்லையெனில் திரும்பு '=';
      }
      நிலை '<': {
        next(ls);
        எனில் (check_next1(ls, '=')) திரும்பு TK_LE;  /* '<=' */
        இல்லையெனில் எனில் (check_next1(ls, '<')) திரும்பு TK_SHL;  /* '<<' */
        இல்லையெனில் திரும்பு '<';
      }
      நிலை '>': {
        next(ls);
        எனில் (check_next1(ls, '=')) திரும்பு TK_GE;  /* '>=' */
        இல்லையெனில் எனில் (check_next1(ls, '>')) திரும்பு TK_SHR;  /* '>>' */
        இல்லையெனில் திரும்பு '>';
      }
      நிலை '/': {
        next(ls);
        எனில் (check_next1(ls, '/')) திரும்பு TK_IDIV;  /* '//' */
        இல்லையெனில் திரும்பு '/';
      }
      நிலை '~': {
        next(ls);
        எனில் (check_next1(ls, '=')) திரும்பு TK_NE;  /* '~=' */
        இல்லையெனில் திரும்பு '~';
      }
      நிலை ':': {
        next(ls);
        எனில் (check_next1(ls, ':')) திரும்பு TK_DBCOLON;  /* '::' */
        இல்லையெனில் திரும்பு ':';
      }
      நிலை '"': நிலை '\'': {  /* short literal strings */
        read_string(ls, ls->current, seminfo);
        திரும்பு TK_STRING;
      }
      நிலை '.': {  /* '.', '..', '...', or number */
        save_and_next(ls);
        எனில் (check_next1(ls, '.')) {
          எனில் (check_next1(ls, '.'))
            திரும்பு TK_DOTS;   /* '...' */
          இல்லையெனில் திரும்பு TK_CONCAT;   /* '..' */
        }
        இல்லையெனில் எனில் (!lisdigit(ls->current)) திரும்பு '.';
        இல்லையெனில் திரும்பு read_numeral(ls, seminfo);
      }
      நிலை '0': நிலை '1': நிலை '2': நிலை '3': நிலை '4':
      நிலை '5': நிலை '6': நிலை '7': நிலை '8': நிலை '9': {
        திரும்பு read_numeral(ls, seminfo);
      }
      நிலை EOZ: {
        திரும்பு TK_EOS;
      }
      இயல்பு: {
        எனில் (lislalpha(ls->current)) {  /* identifier or reserved word? */
          TString *ts;
          செய் {
            save_and_next(ls);
          } போது (lislalnum(ls->current));
          /* find or create string */
          ts = luaS_newlstr(ls->L, luaZ_buffer(ls->buff),
                                   luaZ_bufflen(ls->buff));
          எனில் (isreserved(ts))   /* reserved word? */
            திரும்பு ts->extra - 1 + FIRST_RESERVED;
          இல்லையெனில் {
            seminfo->ts = anchorstr(ls, ts);
            திரும்பு TK_NAME;
          }
        }
        இல்லையெனில் {  /* single-char tokens ('+', '*', '%', '{', '}', ...) */
          முழு c = ls->current;
          next(ls);
          திரும்பு c;
        }
      }
    }
  }
}


வெறுமை luaX_next (LexState *ls) {
  ls->lastline = ls->linenumber;
  எனில் (ls->lookahead.token != TK_EOS) {  /* is there a look-ahead token? */
    ls->t = ls->lookahead;  /* use this one */
    ls->lookahead.token = TK_EOS;  /* and discharge it */
  }
  இல்லையெனில்
    ls->t.token = llex(ls, &ls->t.seminfo);  /* read next token */
}


முழு luaX_lookahead (LexState *ls) {
  lua_assert(ls->lookahead.token == TK_EOS);
  ls->lookahead.token = llex(ls, &ls->lookahead.seminfo);
  திரும்பு ls->lookahead.token;
}

