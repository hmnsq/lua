/*
** $Id: llex.c $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#define llex_c
#define LUA_CORE

#ضمن "lprefix.h"


#ضمن <locale.h>
#ضمن <string.h>

#ضمن "lua.h"

#ضمن "lctype.h"
#ضمن "ldebug.h"
#ضمن "ldo.h"
#ضمن "lgc.h"
#ضمن "llex.h"
#ضمن "lobject.h"
#ضمن "lparser.h"
#ضمن "lstate.h"
#ضمن "lstring.h"
#ضمن "ltable.h"
#ضمن "lzio.h"



#define next(ls)	(ls->current = zgetc(ls->z))


/* minimum size for string buffer */
#إذا !defined(LUA_MINBUFFER)
#define LUA_MINBUFFER   32
#endif


#define currIsNewline(ls)	(ls->current == '\n' || ls->current == '\r')


/* ORDER RESERVED */
ساكن ثابت حرف *ثابت luaX_tokens [] = {
    "and", "break", "do", "else", "elseif",
    "end", "false", "for", "function", "global", "goto", "if",
    "in", "local", "nil", "not", "or", "repeat",
    "return", "then", "true", "until", "while",
    "//", "..", "...", "==", ">=", "<=", "~=",
    "<<", ">>", "::", "<eof>",
    "<number>", "<integer>", "<name>", "<string>"
};


#define save_and_next(ls) (save(ls, ls->current), next(ls))


ساكن l_noret lexerror (LexState *ls, ثابت حرف *رسالة, صحيح token);


ساكن فراغ save (LexState *ls, صحيح c) {
  Mbuffer *b = ls->buff;
  إذا (luaZ_bufflen(b) + 1 > luaZ_sizebuffer(b)) {
    size_t newsize = luaZ_sizebuffer(b);  /* get old size */;
    إذا (newsize >= (MAX_SIZE/3 * 2))  /* larger than MAX_SIZE/1.5 ? */
      lexerror(ls, "lexical element too long", 0);
    newsize += (newsize >> 1);  /* new size is 1.5 times the old one */
    luaZ_resizebuffer(ls->L, b, newsize);
  }
  b->buffer[luaZ_bufflen(b)++] = cast_char(c);
}


فراغ luaX_init (lua_State *L) {
  صحيح i;
  TString *e = luaS_newliteral(L, LUA_ENV);  /* create env name */
  luaC_fix(L, obj2gco(e));  /* never collect this name */
  لكل (i=0; i<رقم_RESERVED; i++) {
    TString *ts = luaS_new(L, luaX_tokens[i]);
    luaC_fix(L, obj2gco(ts));  /* reserved words are never collected */
    ts->extra = cast_byte(i+1);  /* reserved word */
  }
}


ثابت حرف *luaX_token2str (LexState *ls, صحيح token) {
  إذا (token < FIRST_RESERVED) {  /* single-byte symbols? */
    إذا (lisprint(token))
      إرجاع luaO_pushfstring(ls->L, "'%c'", token);
    وإلا  /* control character */
      إرجاع luaO_pushfstring(ls->L, "'<\\%d>'", token);
  }
  وإلا {
    ثابت حرف *s = luaX_tokens[token - FIRST_RESERVED];
    إذا (token < TK_EOS)  /* fixed format (symbols and reserved words)? */
      إرجاع luaO_pushfstring(ls->L, "'%s'", s);
    وإلا  /* names, strings, and numerals */
      إرجاع s;
  }
}


ساكن ثابت حرف *txtToken (LexState *ls, صحيح token) {
  بدل (token) {
    حالة TK_NAME: حالة TK_STRING:
    حالة TK_FLT: حالة TK_INT:
      save(ls, '\0');
      إرجاع luaO_pushfstring(ls->L, "'%s'", luaZ_buffer(ls->buff));
    افتراضي:
      إرجاع luaX_token2str(ls, token);
  }
}


ساكن l_noret lexerror (LexState *ls, ثابت حرف *رسالة, صحيح token) {
  رسالة = luaG_addinfo(ls->L, رسالة, ls->source, ls->linenumber);
  إذا (token)
    luaO_pushfstring(ls->L, "%s near %s", رسالة, txtToken(ls, token));
  luaD_throw(ls->L, LUA_ERRSYNTAX);
}


l_noret luaX_syntaxerror (LexState *ls, ثابت حرف *رسالة) {
  lexerror(ls, رسالة, ls->t.token);
}


/*
** Anchors a string in scanner's table so that it will not be collected
** until the end of the compilation; by that time it should be anchored
** somewhere. It also internalizes long strings, ensuring there is only
** one copy of each unique string.
*/
ساكن TString *anchorstr (LexState *ls, TString *ts) {
  lua_State *L = ls->L;
  TValue oldts;
  صحيح tag = luaH_getstr(ls->h, ts, &oldts);
  إذا (!tagisempty(tag))  /* string already present? */
    إرجاع tsvalue(&oldts);  /* use stored value */
  وإلا {  /* create a new entry */
    TValue *stv = s2v(L->قمة.p++);  /* reserve stack space for string */
    setsvalue(L, stv, ts);  /* push (anchor) the string on the stack */
    luaH_set(L, ls->h, stv, stv);  /* t[string] = string */
    /* table is not a metatable, so it does not need to invalidate cache */
    luaC_checkGC(L);
    L->قمة.p--;  /* remove string from stack */
    إرجاع ts;
  }
}


/*
** Creates a new string and anchors it in scanner's table.
*/
TString *luaX_newstring (LexState *ls, ثابت حرف *str, size_t l) {
  إرجاع anchorstr(ls, luaS_newlstr(ls->L, str, l));
}


/*
** increment line number and skips newline sequence (any of
** \n, \r, \n\r, or \r\n)
*/
ساكن فراغ inclinenumber (LexState *ls) {
  صحيح old = ls->current;
  lua_assert(currIsNewline(ls));
  next(ls);  /* skip '\n' or '\r' */
  إذا (currIsNewline(ls) && ls->current != old)
    next(ls);  /* skip '\n\r' or '\r\n' */
  إذا (++ls->linenumber >= INT_MAX)
    lexerror(ls, "chunk has too many lines", 0);
}


فراغ luaX_setinput (lua_State *L, LexState *ls, ZIO *z, TString *source,
                    صحيح firstchar) {
  ls->t.token = 0;
  ls->L = L;
  ls->current = firstchar;
  ls->lookahead.token = TK_EOS;  /* no look-ahead token */
  ls->z = z;
  ls->fs = عدم;
  ls->linenumber = 1;
  ls->lastline = 1;
  ls->source = source;
  /* all three strings here ("_ENV", "break", "global") were fixed,
     so they cannot be collected */
  ls->envn = luaS_newliteral(L, LUA_ENV);  /* get env string */
  ls->brkn = luaS_newliteral(L, "break");  /* get "break" string */
#إذا LUA_COMPAT_GLOBAL
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


ساكن صحيح check_next1 (LexState *ls, صحيح c) {
  إذا (ls->current == c) {
    next(ls);
    إرجاع 1;
  }
  وإلا إرجاع 0;
}


/*
** Check whether current char is in set 'set' (with two chars) and
** saves it
*/
ساكن صحيح check_next2 (LexState *ls, ثابت حرف *set) {
  lua_assert(set[2] == '\0');
  إذا (ls->current == set[0] || ls->current == set[1]) {
    save_and_next(ls);
    إرجاع 1;
  }
  وإلا إرجاع 0;
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
ساكن صحيح read_numeral (LexState *ls, SemInfo *seminfo) {
  TValue obj;
  ثابت حرف *expo = "Ee";
  صحيح first = ls->current;
  lua_assert(lisdigit(ls->current));
  save_and_next(ls);
  إذا (first == '0' && check_next2(ls, "xX"))  /* hexadecimal? */
    expo = "Pp";
  لكل (;;) {
    إذا (check_next2(ls, expo))  /* exponent mark? */
      check_next2(ls, "-+");  /* optional exponent sign */
    وإلا إذا (lisxdigit(ls->current) || ls->current == '.')  /* '%x|%.' */
      save_and_next(ls);
    وإلا اكسر;
  }
  إذا (lislalpha(ls->current))  /* is numeral touching a letter? */
    save_and_next(ls);  /* force an error */
  save(ls, '\0');
  إذا (luaO_str2num(luaZ_buffer(ls->buff), &obj) == 0)  /* format error? */
    lexerror(ls, "malformed number", TK_FLT);
  إذا (ttisinteger(&obj)) {
    seminfo->i = ivalue(&obj);
    إرجاع TK_INT;
  }
  وإلا {
    lua_assert(ttisfloat(&obj));
    seminfo->r = fltvalue(&obj);
    إرجاع TK_FLT;
  }
}


/*
** read a sequence '[=*[' or ']=*]', leaving the last bracket. If
** sequence is well formed, return its number of '='s + 2; otherwise,
** return 1 if it is a single bracket (no '='s and no 2nd bracket);
** otherwise (an unfinished '[==...') return 0.
*/
ساكن size_t skip_sep (LexState *ls) {
  size_t تعداد_2 = 0;
  صحيح s = ls->current;
  lua_assert(s == '[' || s == ']');
  save_and_next(ls);
  بينما (ls->current == '=') {
    save_and_next(ls);
    تعداد_2++;
  }
  إرجاع (ls->current == s) ? تعداد_2 + 2
         : (تعداد_2 == 0) ? 1
         : 0;
}


ساكن فراغ read_long_string (LexState *ls, SemInfo *seminfo, size_t sep) {
  صحيح سطر = ls->linenumber;  /* initial line (for error message) */
  save_and_next(ls);  /* skip 2nd '[' */
  إذا (currIsNewline(ls))  /* string starts with a newline? */
    inclinenumber(ls);  /* skip it */
  لكل (;;) {
    بدل (ls->current) {
      حالة EOZ: {  /* error */
        ثابت حرف *what = (seminfo ? "string" : "comment");
        ثابت حرف *رسالة = luaO_pushfstring(ls->L,
                     "unfinished long %s (starting at line %d)", what, سطر);
        lexerror(ls, رسالة, TK_EOS);
        اكسر;  /* to avoid warnings */
      }
      حالة ']': {
        إذا (skip_sep(ls) == sep) {
          save_and_next(ls);  /* skip 2nd ']' */
          اذهب endloop;
        }
        اكسر;
      }
      حالة '\n': حالة '\r': {
        save(ls, '\n');
        inclinenumber(ls);
        إذا (!seminfo) luaZ_resetbuffer(ls->buff);  /* avoid wasting space */
        اكسر;
      }
      افتراضي: {
        إذا (seminfo) save_and_next(ls);
        وإلا next(ls);
      }
    }
  } endloop:
  إذا (seminfo)
    seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + sep,
                                     luaZ_bufflen(ls->buff) - 2 * sep);
}


ساكن فراغ esccheck (LexState *ls, صحيح c, ثابت حرف *رسالة) {
  إذا (!c) {
    إذا (ls->current != EOZ)
      save_and_next(ls);  /* add current to buffer for error message */
    lexerror(ls, رسالة, TK_STRING);
  }
}


ساكن صحيح gethexa (LexState *ls) {
  save_and_next(ls);
  esccheck (ls, lisxdigit(ls->current), "hexadecimal digit expected");
  إرجاع luaO_hexavalue(ls->current);
}


ساكن صحيح readhexaesc (LexState *ls) {
  صحيح r = gethexa(ls);
  r = (r << 4) + gethexa(ls);
  luaZ_buffremove(ls->buff, 2);  /* remove saved chars from buffer */
  إرجاع r;
}


/*
** When reading a UTF-8 escape sequence, save everything to the buffer
** for error reporting in case of errors; 'i' counts the number of
** saved characters, so that they can be removed if case of success.
*/
ساكن l_uint32 readutf8esc (LexState *ls) {
  l_uint32 r;
  صحيح i = 4;  /* number of chars to be removed: start with #"\u{X" */
  save_and_next(ls);  /* skip 'u' */
  esccheck(ls, ls->current == '{', "missing '{'");
  r = cast_uint(gethexa(ls));  /* must have at least one digit */
  بينما (cast_void(save_and_next(ls)), lisxdigit(ls->current)) {
    i++;
    esccheck(ls, r <= (0x7FFFFFFFu >> 4), "UTF-8 value too large");
    r = (r << 4) + luaO_hexavalue(ls->current);
  }
  esccheck(ls, ls->current == '}', "missing '}'");
  next(ls);  /* skip '}' */
  luaZ_buffremove(ls->buff, i);  /* remove saved chars from buffer */
  إرجاع r;
}


ساكن فراغ utf8esc (LexState *ls) {
  حرف buff[UTF8BUFFSZ];
  صحيح n = luaO_utf8esc(buff, readutf8esc(ls));
  لكل (; n > 0; n--)  /* add 'buff' to string */
    save(ls, buff[UTF8BUFFSZ - n]);
}


ساكن صحيح readdecesc (LexState *ls) {
  صحيح i;
  صحيح r = 0;  /* result accumulator */
  لكل (i = 0; i < 3 && lisdigit(ls->current); i++) {  /* read up to 3 digits */
    r = 10*r + ls->current - '0';
    save_and_next(ls);
  }
  esccheck(ls, r <= UCHAR_MAX, "decimal escape too large");
  luaZ_buffremove(ls->buff, i);  /* remove read digits from buffer */
  إرجاع r;
}


ساكن فراغ read_string (LexState *ls, صحيح del, SemInfo *seminfo) {
  save_and_next(ls);  /* keep delimiter (for error messages) */
  بينما (ls->current != del) {
    بدل (ls->current) {
      حالة EOZ:
        lexerror(ls, "unfinished string", TK_EOS);
        اكسر;  /* to avoid warnings */
      حالة '\n':
      حالة '\r':
        lexerror(ls, "unfinished string", TK_STRING);
        اكسر;  /* to avoid warnings */
      حالة '\\': {  /* escape sequences */
        صحيح c;  /* final character to be saved */
        save_and_next(ls);  /* keep '\\' for error messages */
        بدل (ls->current) {
          حالة 'a': c = '\a'; اذهب read_save;
          حالة 'b': c = '\b'; اذهب read_save;
          حالة 'f': c = '\f'; اذهب read_save;
          حالة 'n': c = '\n'; اذهب read_save;
          حالة 'r': c = '\r'; اذهب read_save;
          حالة 't': c = '\t'; اذهب read_save;
          حالة 'v': c = '\v'; اذهب read_save;
          حالة 'x': c = readhexaesc(ls); اذهب read_save;
          حالة 'u': utf8esc(ls);  اذهب no_save;
          حالة '\n': حالة '\r':
            inclinenumber(ls); c = '\n'; اذهب only_save;
          حالة '\\': حالة '\"': حالة '\'':
            c = ls->current; اذهب read_save;
          حالة EOZ: اذهب no_save;  /* will raise an error next loop */
          حالة 'z': {  /* zap following span of spaces */
            luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
            next(ls);  /* skip the 'z' */
            بينما (lisspace(ls->current)) {
              إذا (currIsNewline(ls)) inclinenumber(ls);
              وإلا next(ls);
            }
            اذهب no_save;
          }
          افتراضي: {
            esccheck(ls, lisdigit(ls->current), "invalid escape sequence");
            c = readdecesc(ls);  /* digital escape '\ddd' */
            اذهب only_save;
          }
        }
       read_save:
         next(ls);
         /* go through */
       only_save:
         luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
         save(ls, c);
         /* go through */
       no_save: اكسر;
      }
      افتراضي:
        save_and_next(ls);
    }
  }
  save_and_next(ls);  /* skip delimiter */
  seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + 1,
                                   luaZ_bufflen(ls->buff) - 2);
}


ساكن صحيح llex (LexState *ls, SemInfo *seminfo) {
  luaZ_resetbuffer(ls->buff);
  لكل (;;) {
    بدل (ls->current) {
      حالة '\n': حالة '\r': {  /* line breaks */
        inclinenumber(ls);
        اكسر;
      }
      حالة ' ': حالة '\f': حالة '\t': حالة '\v': {  /* spaces */
        next(ls);
        اكسر;
      }
      حالة '-': {  /* '-' or '--' (comment) */
        next(ls);
        إذا (ls->current != '-') إرجاع '-';
        /* else is a comment */
        next(ls);
        إذا (ls->current == '[') {  /* long comment? */
          size_t sep = skip_sep(ls);
          luaZ_resetbuffer(ls->buff);  /* 'skip_sep' may dirty the buffer */
          إذا (sep >= 2) {
            read_long_string(ls, عدم, sep);  /* skip long comment */
            luaZ_resetbuffer(ls->buff);  /* previous call may dirty the buff. */
            اكسر;
          }
        }
        /* else short comment */
        بينما (!currIsNewline(ls) && ls->current != EOZ)
          next(ls);  /* skip until end of line (or end of file) */
        اكسر;
      }
      حالة '[': {  /* long string or simply '[' */
        size_t sep = skip_sep(ls);
        إذا (sep >= 2) {
          read_long_string(ls, seminfo, sep);
          إرجاع TK_STRING;
        }
        وإلا إذا (sep == 0)  /* '[=...' missing second bracket? */
          lexerror(ls, "invalid long string delimiter", TK_STRING);
        إرجاع '[';
      }
      حالة '=': {
        next(ls);
        إذا (check_next1(ls, '=')) إرجاع TK_EQ;  /* '==' */
        وإلا إرجاع '=';
      }
      حالة '<': {
        next(ls);
        إذا (check_next1(ls, '=')) إرجاع TK_LE;  /* '<=' */
        وإلا إذا (check_next1(ls, '<')) إرجاع TK_SHL;  /* '<<' */
        وإلا إرجاع '<';
      }
      حالة '>': {
        next(ls);
        إذا (check_next1(ls, '=')) إرجاع TK_GE;  /* '>=' */
        وإلا إذا (check_next1(ls, '>')) إرجاع TK_SHR;  /* '>>' */
        وإلا إرجاع '>';
      }
      حالة '/': {
        next(ls);
        إذا (check_next1(ls, '/')) إرجاع TK_IDIV;  /* '//' */
        وإلا إرجاع '/';
      }
      حالة '~': {
        next(ls);
        إذا (check_next1(ls, '=')) إرجاع TK_NE;  /* '~=' */
        وإلا إرجاع '~';
      }
      حالة ':': {
        next(ls);
        إذا (check_next1(ls, ':')) إرجاع TK_DBCOLON;  /* '::' */
        وإلا إرجاع ':';
      }
      حالة '"': حالة '\'': {  /* short literal strings */
        read_string(ls, ls->current, seminfo);
        إرجاع TK_STRING;
      }
      حالة '.': {  /* '.', '..', '...', or number */
        save_and_next(ls);
        إذا (check_next1(ls, '.')) {
          إذا (check_next1(ls, '.'))
            إرجاع TK_DOTS;   /* '...' */
          وإلا إرجاع TK_CONCAT;   /* '..' */
        }
        وإلا إذا (!lisdigit(ls->current)) إرجاع '.';
        وإلا إرجاع read_numeral(ls, seminfo);
      }
      حالة '0': حالة '1': حالة '2': حالة '3': حالة '4':
      حالة '5': حالة '6': حالة '7': حالة '8': حالة '9': {
        إرجاع read_numeral(ls, seminfo);
      }
      حالة EOZ: {
        إرجاع TK_EOS;
      }
      افتراضي: {
        إذا (lislalpha(ls->current)) {  /* identifier or reserved word? */
          TString *ts;
          افعل {
            save_and_next(ls);
          } بينما (lislalnum(ls->current));
          /* find or create string */
          ts = luaS_newlstr(ls->L, luaZ_buffer(ls->buff),
                                   luaZ_bufflen(ls->buff));
          إذا (isreserved(ts))   /* reserved word? */
            إرجاع ts->extra - 1 + FIRST_RESERVED;
          وإلا {
            seminfo->ts = anchorstr(ls, ts);
            إرجاع TK_NAME;
          }
        }
        وإلا {  /* single-char tokens ('+', '*', '%', '{', '}', ...) */
          صحيح c = ls->current;
          next(ls);
          إرجاع c;
        }
      }
    }
  }
}


فراغ luaX_next (LexState *ls) {
  ls->lastline = ls->linenumber;
  إذا (ls->lookahead.token != TK_EOS) {  /* is there a look-ahead token? */
    ls->t = ls->lookahead;  /* use this one */
    ls->lookahead.token = TK_EOS;  /* and discharge it */
  }
  وإلا
    ls->t.token = llex(ls, &ls->t.seminfo);  /* read next token */
}


صحيح luaX_lookahead (LexState *ls) {
  lua_assert(ls->lookahead.token == TK_EOS);
  ls->lookahead.token = llex(ls, &ls->lookahead.seminfo);
  إرجاع ls->lookahead.token;
}

