/*
 * Modification History
 *
 * 2002-May-2   Jason Rohrer
 * Created.
 *
 * 2002-May-5   Jason Rohrer
 * Added support for the request and entry form url format.
 */



#ifndef THOUGHT_LINKS_PAGE_GENERATOR_INCLUDED
#define THOUGHT_LINKS_PAGE_GENERATOR_INCLUDED



#include "PageGenerator.h"



#include "minorGems/io/OutputStream.h"



/**
 * A thoughtLink implementation of an HTML page generator.
 *
 * @author Jason Rohrer
 */
class ThoughtLinksPageGenerator : public PageGenerator {

    public:



        ThoughtLinksPageGenerator();


        
        ~ThoughtLinksPageGenerator();
        
                
        
        // implements the PageGenerator interface
        void generatePage( char *inGetRequestPath,
                           OutputStream *inOutputStream );

        char *getMimeType( char *inGetRequestPath );


        
    protected:


        /**
         * Loads the contents of a template file from the current
         * skin directory into a string.
         *
         * @param inFileName the name of the file to load (without path),
         *   as a \0-terminated string.
         *   Must be destroyed by caller.
         *
         * @return the contents of the file as a \0-terminated string.
         *   Must be destroyed by caller.
         */
        char *loadStringFromTemplateFile( char *inFileName );



        /**
         * Replaces the first occurrence of a target string with
         * a substitute string.
         *
         * All parameters and return value must be destroyed by caller.
         *
         * @param inHaystack the string to search for inTarget in. 
         * @param inTarget the string to search for.
         * @param inSubstitute the string to replace the first occurrence
         *   of the target with.
         * @param outFound a pre-allocated character which will be filled
         *   with true if the target is found, and filled with false
         *   otherwise.
         *
         * @return a newly allocated string with the substitution performed.
         */
        char *replaceOnce( char *inHaystack, char *inTarget,
                           char *inSubstitute,
                           char *outFound );



        /**
         * Replaces the all occurrences of a target string with
         * a substitute string.
         *
         * Note that this function is not self-insertion-safe:
         * If inSubstitute contains inTarget, this function will
         * enter an infinite loop.
         *         
         * All parameters and return value must be destroyed by caller.
         *
         * @param inHaystack the string to search for inTarget in. 
         * @param inTarget the string to search for.
         * @param inSubstitute the string to replace the all occurrences
         *   of the target with.
         * @param outFound a pre-allocated character which will be filled
         *   with true if the target is found at least once,
         *   and filled with false otherwise.
         *
         * @return a newly allocated string with the substitutions performed.
         */
        char *replaceAll( char *inHaystack, char *inTarget,
                           char *inSubstitute,
                           char *outFound );



        /**
         * Extracts the portion of a string up to but no including the
         * first occurrence of a target substring.
         *
         * All parameters and return value must be destroyed by caller.
         *
         * @param inHaystack the string to search for inTarget in. 
         * @param inTarget the string to search for.
         * @param outFound a pre-allocated character which will be filled
         *   with true if the target is found at least once,
         *   and filled with false otherwise.
         *
         * @return a newly allocated string containing a prefix
         *   of inHaystack up to but not including the first occurrence
         *   of inTarget.
         */         
        char *extractUpToTarget(
            char *inHaystack, char *inTarget, char *outFound );



        /**
         * Extracts the portion of a string after but no including the
         * first occurrence of a target substring.
         *
         * If the target does not exist, this function returns
         * the empty string "\0".
         *
         * All parameters and return value must be destroyed by caller.
         *
         * @param inHaystack the string to search for inTarget in. 
         * @param inTarget the string to search for.
         * @param outFound a pre-allocated character which will be filled
         *   with true if the target is found at least once,
         *   and filled with false otherwise.
         *
         * @return a newly allocated string containing a suffix
         *   of inHaystack after but not including the first occurrence
         *   of inTarget.
         */         
        char *extractAfterTarget(
            char *inHaystack, char *inTarget, char *outFound );

        

        /**
         * Extracts the value from an argument of the form:
         * name=value&   or
         * name=value[string_end]
         *
         * Note that if name is the suffix of an argument with a longer name
         * the longer-named argument's value may be returned.  Thus,
         * argument names should not be suffixes of eachother.
         *
         * All parameters must be destroyed by caller.
         *
         * @param inHaystack the string to extract the argument from.
         * @param inArgName the argument name (without ?, &, or =) to search
         *   for in inHaystack.
         *
         * @return the value of the argument, or NULL if the argument is
         * not found.
         */
        char *extractArgument( char *inHaystack,
                               char *inArgName );



        /**
         * The same as extractArgument, except that explicit
         * hex representations are translated to plain ascii
         * before the argument value is returned.
         */
        char *extractArgumentRemoveHex( char *inHaystack,
                                        char *inArgName );


        
        void generatePageHead( OutputStream *inOutputStream );


        
        void generateEntryForm(
            char *inGetRequestPath,
            OutputStream *inOutputStream );


        
        void generateRequestPage(
            char *inGetRequestPath,
            OutputStream *inOutputStream );



        /**
         * Replaces all relative links (for example, image links)
         * with absolute links.
         *
         * @param inPageString the body of the html page as a \0-terminated
         *   string.  Must be destroyed by caller.
         * @param inPageURL the url of this page as a \0-terminated string.
         *   Must be destroyed by caller.
         *
         * @return a newly allocated \0-terminated string containing
         *   the page with the replacements.
         */
        char *fixAllRelativeLinks( char *inPageString,
                                   char *inPageURL );


        
        /**
         * Replaces relative links (for example, image src links)
         * with absolute links.
         *
         * @param inAnchorLowercase the lowercase version of the anchor
         *   (for example, "src=\"" )
         *   as a \0-terminated string.  Must be destroyed by caller.
         * @param inAnchorUppercase the uppercase version of the anchor
         *   (for example, "SRC=\"" )
         *   as a \0-terminated string.  Must be destroyed by caller.
         * @param inPageString the body of the html page as a \0-terminated
         *   string.  Must be destroyed by caller.
         * @param inPageURL the url of this page as a \0-terminated string.
         *   Must be destroyed by caller.
         *
         * @return a newly allocated \0-terminated string containing
         *   the page with the replacements.
         */
        char *makeRelativeLinksAbsolute( char *inAnchorLowercase,
                                         char *inAnchorUppercase,
                                         char *inPageString,
                                         char *inPageURL );

        
        /**
         * inString and returned string must be destroyed by caller.
         */
        char *removeExplicitHex( char *inString );



        /**
         * Encodes a string in a browser-safe hex encoding
         * (including adding '+' for each space).
         *
         * @param inString the string to encode.
         *   Must be destroyed by caller.
         *
         * @return an encoded string.  Must be destroyed by caller.
         */
        char *hexEncode( char *inString );


        
    };



#endif
