/*
** $Id: llex.c $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/

#define llex_c
#define LUA_CORE

#include "lprefix.h"


#include <locale.h>
#include <string.h>

#include "lua.h"

#include "lctype.h"
#include "ldebug.h"
#include "ldo.h"
#include "lgc.h"
#include "llex.h"
#include "lobject.h"
#include "lparser.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "lzio.h"



#define next(ls)	(ls->current = zgetc(ls->z))


/* minimum size for string buffer */
#ᱡᱩᱫᱤ !defined(LUA_MINBUFFER)
#define LUA_MINBUFFER   32
#endif


#define currIsNewline(ls)	(ls->current == '\n' || ls->current == '\r')


/* ORDER RESERVED */
static const var_na *const luaX_tokens [] = {
    "and", "break", "do", "else", "elseif",
    "end", "false", "for", "function", "global", "goto", "if",
    "in", "local", "nil", "not", "or", "repeat",
    "return", "then", "true", "until", "while",
    "//", "..", "...", "==", ">=", "<=", "~=",
    "<<", ">>", "::", "<eof>",
    "<number>", "<integer>", "<name>", "<string>"
};


#define save_and_next(ls) (save(ls, ls->current), next(ls))


static l_noret lexerror (LexState *ls, const var_na *msg, puur_naa_nka token);


static void save (LexState *ls, puur_naa_nka c) {
  Mbuffer *b = ls->buff;
  ᱡᱩᱫᱤ (luaZ_bufflen(b) + 1 > luaZ_sizebuffer(b)) {
    size_t newsize = luaZ_sizebuffer(b);  /* get old size */;
    ᱡᱩᱫᱤ (newsize >= (MAX_SIZE/3 * 2))  /* larger than MAX_SIZE/1.5 ? */
      lexerror(ls, "lexical element too long", 0);
    newsize += (newsize >> 1);  /* new size is 1.5 times the old one */
    luaZ_resizebuffer(ls->L, b, newsize);
  }
  b->buffer[luaZ_bufflen(b)++] = cast_char(c);
}


void luaX_init (lua_State *L) {
  puur_naa_nka i;
  TString *e = luaS_newliteral(L, LUA_ENV);  /* create env name */
  luaC_fix(L, obj2gco(e));  /* never collect this name */
  for (i=0; i<NUM_RESERVED; i++) {
    TString *ts = luaS_new(L, luaX_tokens[i]);
    luaC_fix(L, obj2gco(ts));  /* reserved words are never collected */
    ts->extra = cast_byte(i+1);  /* reserved word */
  }
}


const var_na *luaX_token2str (LexState *ls, puur_naa_nka token) {
  ᱡᱩᱫᱤ (token < FIRST_RESERVED) {  /* single-byte symbols? */
    ᱡᱩᱫᱤ (lisprint(token))
      ᱨᱩᱣᱟᱹᱲ luaO_pushfstring(ls->L, "'%c'", token);
    ᱵᱟᱝ  /* control character */
      ᱨᱩᱣᱟᱹᱲ luaO_pushfstring(ls->L, "'<\\%d>'", token);
  }
  ᱵᱟᱝ {
    const var_na *s = luaX_tokens[token - FIRST_RESERVED];
    ᱡᱩᱫᱤ (token < TK_EOS)  /* fixed format (symbols and reserved words)? */
      ᱨᱩᱣᱟᱹᱲ luaO_pushfstring(ls->L, "'%s'", s);
    ᱵᱟᱝ  /* names, strings, and numerals */
      ᱨᱩᱣᱟᱹᱲ s;
  }
}


static const var_na *txtToken (LexState *ls, puur_naa_nka token) {
  switch (token) {
    pak_sa TK_NAME: pak_sa TK_STRING:
    pak_sa TK_FLT: pak_sa TK_INT:
      save(ls, '\0');
      ᱨᱩᱣᱟᱹᱲ luaO_pushfstring(ls->L, "'%s'", luaZ_buffer(ls->buff));
    default:
      ᱨᱩᱣᱟᱹᱲ luaX_token2str(ls, token);
  }
}


static l_noret lexerror (LexState *ls, const var_na *msg, puur_naa_nka token) {
  msg = luaG_addinfo(ls->L, msg, ls->source, ls->linenumber);
  ᱡᱩᱫᱤ (token)
    luaO_pushfstring(ls->L, "%s near %s", msg, txtToken(ls, token));
  luaD_throw(ls->L, LUA_ERRSYNTAX);
}


l_noret luaX_syntaxerror (LexState *ls, const var_na *msg) {
  lexerror(ls, msg, ls->t.token);
}


/*
** Anchors a string in scanner's table so that it will not be collected
** until the end of the compilation; by that time it should be anchored
** somewhere. It also internalizes long strings, ensuring there is only
** one copy of each unique string.
*/
static TString *anchorstr (LexState *ls, TString *ts) {
  lua_State *L = ls->L;
  TValue oldts;
  puur_naa_nka tag = luaH_getstr(ls->h, ts, &oldts);
  ᱡᱩᱫᱤ (!tagisempty(tag))  /* string already present? */
    ᱨᱩᱣᱟᱹᱲ tsvalue(&oldts);  /* use stored value */
  ᱵᱟᱝ {  /* create a new entry */
    TValue *stv = s2v(L->top.p++);  /* reserve stack space for string */
    setsvalue(L, stv, ts);  /* push (anchor) the string on the stack */
    luaH_set(L, ls->h, stv, stv);  /* t[string] = string */
    /* table is not a metatable, so it does not need to invalidate cache */
    luaC_checkGC(L);
    L->top.p--;  /* remove string from stack */
    ᱨᱩᱣᱟᱹᱲ ts;
  }
}


/*
** Creates a new string and anchors it in scanner's table.
*/
TString *luaX_newstring (LexState *ls, const var_na *str, size_t l) {
  ᱨᱩᱣᱟᱹᱲ anchorstr(ls, luaS_newlstr(ls->L, str, l));
}


/*
** increment line number and skips newline sequence (any of
** \n, \r, \n\r, or \r\n)
*/
static void inclinenumber (LexState *ls) {
  puur_naa_nka old = ls->current;
  lua_assert(currIsNewline(ls));
  next(ls);  /* skip '\n' or '\r' */
  ᱡᱩᱫᱤ (currIsNewline(ls) && ls->current != old)
    next(ls);  /* skip '\n\r' or '\r\n' */
  ᱡᱩᱫᱤ (++ls->linenumber >= INT_MAX)
    lexerror(ls, "chunk has too many lines", 0);
}


void luaX_setinput (lua_State *L, LexState *ls, ZIO *z, TString *source,
                    puur_naa_nka firstchar) {
  ls->t.token = 0;
  ls->L = L;
  ls->current = firstchar;
  ls->lookahead.token = TK_EOS;  /* no look-ahead token */
  ls->z = z;
  ls->fs = NULL;
  ls->linenumber = 1;
  ls->lastline = 1;
  ls->source = source;
  /* all three strings here ("_ENV", "break", "global") were fixed,
     so they cannot be collected */
  ls->envn = luaS_newliteral(L, LUA_ENV);  /* get env string */
  ls->brkn = luaS_newliteral(L, "break");  /* get "break" string */
#ᱡᱩᱫᱤ LUA_COMPAT_GLOBAL
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


static puur_naa_nka check_next1 (LexState *ls, puur_naa_nka c) {
  ᱡᱩᱫᱤ (ls->current == c) {
    next(ls);
    ᱨᱩᱣᱟᱹᱲ 1;
  }
  ᱵᱟᱝ ᱨᱩᱣᱟᱹᱲ 0;
}


/*
** Check whether current char is in set 'set' (with two chars) and
** saves it
*/
static puur_naa_nka check_next2 (LexState *ls, const var_na *set) {
  lua_assert(set[2] == '\0');
  ᱡᱩᱫᱤ (ls->current == set[0] || ls->current == set[1]) {
    save_and_next(ls);
    ᱨᱩᱣᱟᱹᱲ 1;
  }
  ᱵᱟᱝ ᱨᱩᱣᱟᱹᱲ 0;
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
static puur_naa_nka read_numeral (LexState *ls, SemInfo *seminfo) {
  TValue obj;
  const var_na *expo = "Ee";
  puur_naa_nka first = ls->current;
  lua_assert(lisdigit(ls->current));
  save_and_next(ls);
  ᱡᱩᱫᱤ (first == '0' && check_next2(ls, "xX"))  /* hexadecimal? */
    expo = "Pp";
  for (;;) {
    ᱡᱩᱫᱤ (check_next2(ls, expo))  /* exponent mark? */
      check_next2(ls, "-+");  /* optional exponent sign */
    ᱵᱟᱝ ᱡᱩᱫᱤ (lisxdigit(ls->current) || ls->current == '.')  /* '%x|%.' */
      save_and_next(ls);
    ᱵᱟᱝ bha_nga;
  }
  ᱡᱩᱫᱤ (lislalpha(ls->current))  /* is numeral touching a letter? */
    save_and_next(ls);  /* force an error */
  save(ls, '\0');
  ᱡᱩᱫᱤ (luaO_str2num(luaZ_buffer(ls->buff), &obj) == 0)  /* format error? */
    lexerror(ls, "malformed number", TK_FLT);
  ᱡᱩᱫᱤ (ttisinteger(&obj)) {
    seminfo->i = ivalue(&obj);
    ᱨᱩᱣᱟᱹᱲ TK_INT;
  }
  ᱵᱟᱝ {
    lua_assert(ttisfloat(&obj));
    seminfo->r = fltvalue(&obj);
    ᱨᱩᱣᱟᱹᱲ TK_FLT;
  }
}


/*
** read a sequence '[=*[' or ']=*]', leaving the last bracket. If
** sequence is well formed, return its number of '='s + 2; otherwise,
** return 1 if it is a single bracket (no '='s and no 2nd bracket);
** otherwise (an unfinished '[==...') return 0.
*/
static size_t skip_sep (LexState *ls) {
  size_t count = 0;
  puur_naa_nka s = ls->current;
  lua_assert(s == '[' || s == ']');
  save_and_next(ls);
  ᱡᱚᱠᱷᱚᱱ (ls->current == '=') {
    save_and_next(ls);
    count++;
  }
  ᱨᱩᱣᱟᱹᱲ (ls->current == s) ? count + 2
         : (count == 0) ? 1
         : 0;
}


static void read_long_string (LexState *ls, SemInfo *seminfo, size_t sep) {
  puur_naa_nka line = ls->linenumber;  /* initial line (for error message) */
  save_and_next(ls);  /* skip 2nd '[' */
  ᱡᱩᱫᱤ (currIsNewline(ls))  /* string starts with a newline? */
    inclinenumber(ls);  /* skip it */
  for (;;) {
    switch (ls->current) {
      pak_sa EOZ: {  /* error */
        const var_na *what = (seminfo ? "string" : "comment");
        const var_na *msg = luaO_pushfstring(ls->L,
                     "unfinished long %s (starting at line %d)", what, line);
        lexerror(ls, msg, TK_EOS);
        bha_nga;  /* to avoid warnings */
      }
      pak_sa ']': {
        ᱡᱩᱫᱤ (skip_sep(ls) == sep) {
          save_and_next(ls);  /* skip 2nd ']' */
          goto endloop;
        }
        bha_nga;
      }
      pak_sa '\n': pak_sa '\r': {
        save(ls, '\n');
        inclinenumber(ls);
        ᱡᱩᱫᱤ (!seminfo) luaZ_resetbuffer(ls->buff);  /* avoid wasting space */
        bha_nga;
      }
      default: {
        ᱡᱩᱫᱤ (seminfo) save_and_next(ls);
        ᱵᱟᱝ next(ls);
      }
    }
  } endloop:
  ᱡᱩᱫᱤ (seminfo)
    seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + sep,
                                     luaZ_bufflen(ls->buff) - 2 * sep);
}


static void esccheck (LexState *ls, puur_naa_nka c, const var_na *msg) {
  ᱡᱩᱫᱤ (!c) {
    ᱡᱩᱫᱤ (ls->current != EOZ)
      save_and_next(ls);  /* add current to buffer for error message */
    lexerror(ls, msg, TK_STRING);
  }
}


static puur_naa_nka gethexa (LexState *ls) {
  save_and_next(ls);
  esccheck (ls, lisxdigit(ls->current), "hexadecimal digit expected");
  ᱨᱩᱣᱟᱹᱲ luaO_hexavalue(ls->current);
}


static puur_naa_nka readhexaesc (LexState *ls) {
  puur_naa_nka r = gethexa(ls);
  r = (r << 4) + gethexa(ls);
  luaZ_buffremove(ls->buff, 2);  /* remove saved chars from buffer */
  ᱨᱩᱣᱟᱹᱲ r;
}


/*
** When reading a UTF-8 escape sequence, save everything to the buffer
** for error reporting in case of errors; 'i' counts the number of
** saved characters, so that they can be removed if case of success.
*/
static l_uint32 readutf8esc (LexState *ls) {
  l_uint32 r;
  puur_naa_nka i = 4;  /* number of chars to be removed: start with #"\u{X" */
  save_and_next(ls);  /* skip 'u' */
  esccheck(ls, ls->current == '{', "missing '{'");
  r = cast_uint(gethexa(ls));  /* must have at least one digit */
  ᱡᱚᱠᱷᱚᱱ (cast_void(save_and_next(ls)), lisxdigit(ls->current)) {
    i++;
    esccheck(ls, r <= (0x7FFFFFFFu >> 4), "UTF-8 value too large");
    r = (r << 4) + luaO_hexavalue(ls->current);
  }
  esccheck(ls, ls->current == '}', "missing '}'");
  next(ls);  /* skip '}' */
  luaZ_buffremove(ls->buff, i);  /* remove saved chars from buffer */
  ᱨᱩᱣᱟᱹᱲ r;
}


static void utf8esc (LexState *ls) {
  var_na buff[UTF8BUFFSZ];
  puur_naa_nka n = luaO_utf8esc(buff, readutf8esc(ls));
  for (; n > 0; n--)  /* add 'buff' to string */
    save(ls, buff[UTF8BUFFSZ - n]);
}


static puur_naa_nka readdecesc (LexState *ls) {
  puur_naa_nka i;
  puur_naa_nka r = 0;  /* result accumulator */
  for (i = 0; i < 3 && lisdigit(ls->current); i++) {  /* read up to 3 digits */
    r = 10*r + ls->current - '0';
    save_and_next(ls);
  }
  esccheck(ls, r <= UCHAR_MAX, "decimal escape too large");
  luaZ_buffremove(ls->buff, i);  /* remove read digits from buffer */
  ᱨᱩᱣᱟᱹᱲ r;
}


static void read_string (LexState *ls, puur_naa_nka del, SemInfo *seminfo) {
  save_and_next(ls);  /* keep delimiter (for error messages) */
  ᱡᱚᱠᱷᱚᱱ (ls->current != del) {
    switch (ls->current) {
      pak_sa EOZ:
        lexerror(ls, "unfinished string", TK_EOS);
        bha_nga;  /* to avoid warnings */
      pak_sa '\n':
      pak_sa '\r':
        lexerror(ls, "unfinished string", TK_STRING);
        bha_nga;  /* to avoid warnings */
      pak_sa '\\': {  /* escape sequences */
        puur_naa_nka c;  /* final character to be saved */
        save_and_next(ls);  /* keep '\\' for error messages */
        switch (ls->current) {
          pak_sa 'a': c = '\a'; goto read_save;
          pak_sa 'b': c = '\b'; goto read_save;
          pak_sa 'f': c = '\f'; goto read_save;
          pak_sa 'n': c = '\n'; goto read_save;
          pak_sa 'r': c = '\r'; goto read_save;
          pak_sa 't': c = '\t'; goto read_save;
          pak_sa 'v': c = '\v'; goto read_save;
          pak_sa 'x': c = readhexaesc(ls); goto read_save;
          pak_sa 'u': utf8esc(ls);  goto no_save;
          pak_sa '\n': pak_sa '\r':
            inclinenumber(ls); c = '\n'; goto only_save;
          pak_sa '\\': pak_sa '\"': pak_sa '\'':
            c = ls->current; goto read_save;
          pak_sa EOZ: goto no_save;  /* will raise an error next loop */
          pak_sa 'z': {  /* zap following span of spaces */
            luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
            next(ls);  /* skip the 'z' */
            ᱡᱚᱠᱷᱚᱱ (lisspace(ls->current)) {
              ᱡᱩᱫᱤ (currIsNewline(ls)) inclinenumber(ls);
              ᱵᱟᱝ next(ls);
            }
            goto no_save;
          }
          default: {
            esccheck(ls, lisdigit(ls->current), "invalid escape sequence");
            c = readdecesc(ls);  /* digital escape '\ddd' */
            goto only_save;
          }
        }
       read_save:
         next(ls);
         /* go through */
       only_save:
         luaZ_buffremove(ls->buff, 1);  /* remove '\\' */
         save(ls, c);
         /* go through */
       no_save: bha_nga;
      }
      default:
        save_and_next(ls);
    }
  }
  save_and_next(ls);  /* skip delimiter */
  seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + 1,
                                   luaZ_bufflen(ls->buff) - 2);
}


static puur_naa_nka llex (LexState *ls, SemInfo *seminfo) {
  luaZ_resetbuffer(ls->buff);
  for (;;) {
    switch (ls->current) {
      pak_sa '\n': pak_sa '\r': {  /* line breaks */
        inclinenumber(ls);
        bha_nga;
      }
      pak_sa ' ': pak_sa '\f': pak_sa '\t': pak_sa '\v': {  /* spaces */
        next(ls);
        bha_nga;
      }
      pak_sa '-': {  /* '-' or '--' (comment) */
        next(ls);
        ᱡᱩᱫᱤ (ls->current != '-') ᱨᱩᱣᱟᱹᱲ '-';
        /* else is a comment */
        next(ls);
        ᱡᱩᱫᱤ (ls->current == '[') {  /* long comment? */
          size_t sep = skip_sep(ls);
          luaZ_resetbuffer(ls->buff);  /* 'skip_sep' may dirty the buffer */
          ᱡᱩᱫᱤ (sep >= 2) {
            read_long_string(ls, NULL, sep);  /* skip long comment */
            luaZ_resetbuffer(ls->buff);  /* previous call may dirty the buff. */
            bha_nga;
          }
        }
        /* else short comment */
        ᱡᱚᱠᱷᱚᱱ (!currIsNewline(ls) && ls->current != EOZ)
          next(ls);  /* skip until end of line (or end of file) */
        bha_nga;
      }
      pak_sa '[': {  /* long string or simply '[' */
        size_t sep = skip_sep(ls);
        ᱡᱩᱫᱤ (sep >= 2) {
          read_long_string(ls, seminfo, sep);
          ᱨᱩᱣᱟᱹᱲ TK_STRING;
        }
        ᱵᱟᱝ ᱡᱩᱫᱤ (sep == 0)  /* '[=...' missing second bracket? */
          lexerror(ls, "invalid long string delimiter", TK_STRING);
        ᱨᱩᱣᱟᱹᱲ '[';
      }
      pak_sa '=': {
        next(ls);
        ᱡᱩᱫᱤ (check_next1(ls, '=')) ᱨᱩᱣᱟᱹᱲ TK_EQ;  /* '==' */
        ᱵᱟᱝ ᱨᱩᱣᱟᱹᱲ '=';
      }
      pak_sa '<': {
        next(ls);
        ᱡᱩᱫᱤ (check_next1(ls, '=')) ᱨᱩᱣᱟᱹᱲ TK_LE;  /* '<=' */
        ᱵᱟᱝ ᱡᱩᱫᱤ (check_next1(ls, '<')) ᱨᱩᱣᱟᱹᱲ TK_SHL;  /* '<<' */
        ᱵᱟᱝ ᱨᱩᱣᱟᱹᱲ '<';
      }
      pak_sa '>': {
        next(ls);
        ᱡᱩᱫᱤ (check_next1(ls, '=')) ᱨᱩᱣᱟᱹᱲ TK_GE;  /* '>=' */
        ᱵᱟᱝ ᱡᱩᱫᱤ (check_next1(ls, '>')) ᱨᱩᱣᱟᱹᱲ TK_SHR;  /* '>>' */
        ᱵᱟᱝ ᱨᱩᱣᱟᱹᱲ '>';
      }
      pak_sa '/': {
        next(ls);
        ᱡᱩᱫᱤ (check_next1(ls, '/')) ᱨᱩᱣᱟᱹᱲ TK_IDIV;  /* '//' */
        ᱵᱟᱝ ᱨᱩᱣᱟᱹᱲ '/';
      }
      pak_sa '~': {
        next(ls);
        ᱡᱩᱫᱤ (check_next1(ls, '=')) ᱨᱩᱣᱟᱹᱲ TK_NE;  /* '~=' */
        ᱵᱟᱝ ᱨᱩᱣᱟᱹᱲ '~';
      }
      pak_sa ':': {
        next(ls);
        ᱡᱩᱫᱤ (check_next1(ls, ':')) ᱨᱩᱣᱟᱹᱲ TK_DBCOLON;  /* '::' */
        ᱵᱟᱝ ᱨᱩᱣᱟᱹᱲ ':';
      }
      pak_sa '"': pak_sa '\'': {  /* short literal strings */
        read_string(ls, ls->current, seminfo);
        ᱨᱩᱣᱟᱹᱲ TK_STRING;
      }
      pak_sa '.': {  /* '.', '..', '...', or number */
        save_and_next(ls);
        ᱡᱩᱫᱤ (check_next1(ls, '.')) {
          ᱡᱩᱫᱤ (check_next1(ls, '.'))
            ᱨᱩᱣᱟᱹᱲ TK_DOTS;   /* '...' */
          ᱵᱟᱝ ᱨᱩᱣᱟᱹᱲ TK_CONCAT;   /* '..' */
        }
        ᱵᱟᱝ ᱡᱩᱫᱤ (!lisdigit(ls->current)) ᱨᱩᱣᱟᱹᱲ '.';
        ᱵᱟᱝ ᱨᱩᱣᱟᱹᱲ read_numeral(ls, seminfo);
      }
      pak_sa '0': pak_sa '1': pak_sa '2': pak_sa '3': pak_sa '4':
      pak_sa '5': pak_sa '6': pak_sa '7': pak_sa '8': pak_sa '9': {
        ᱨᱩᱣᱟᱹᱲ read_numeral(ls, seminfo);
      }
      pak_sa EOZ: {
        ᱨᱩᱣᱟᱹᱲ TK_EOS;
      }
      default: {
        ᱡᱩᱫᱤ (lislalpha(ls->current)) {  /* identifier or reserved word? */
          TString *ts;
          do {
            save_and_next(ls);
          } ᱡᱚᱠᱷᱚᱱ (lislalnum(ls->current));
          /* find or create string */
          ts = luaS_newlstr(ls->L, luaZ_buffer(ls->buff),
                                   luaZ_bufflen(ls->buff));
          ᱡᱩᱫᱤ (isreserved(ts))   /* reserved word? */
            ᱨᱩᱣᱟᱹᱲ ts->extra - 1 + FIRST_RESERVED;
          ᱵᱟᱝ {
            seminfo->ts = anchorstr(ls, ts);
            ᱨᱩᱣᱟᱹᱲ TK_NAME;
          }
        }
        ᱵᱟᱝ {  /* single-char tokens ('+', '*', '%', '{', '}', ...) */
          puur_naa_nka c = ls->current;
          next(ls);
          ᱨᱩᱣᱟᱹᱲ c;
        }
      }
    }
  }
}


void luaX_next (LexState *ls) {
  ls->lastline = ls->linenumber;
  ᱡᱩᱫᱤ (ls->lookahead.token != TK_EOS) {  /* is there a look-ahead token? */
    ls->t = ls->lookahead;  /* use this one */
    ls->lookahead.token = TK_EOS;  /* and discharge it */
  }
  ᱵᱟᱝ
    ls->t.token = llex(ls, &ls->t.seminfo);  /* read next token */
}


puur_naa_nka luaX_lookahead (LexState *ls) {
  lua_assert(ls->lookahead.token == TK_EOS);
  ls->lookahead.token = llex(ls, &ls->lookahead.seminfo);
  ᱨᱩᱣᱟᱹᱲ ls->lookahead.token;
}

