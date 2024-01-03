/*! \file
 *
 * MockNetworkAccessManager
 * https://gitlab.com/julrich/MockNetworkAccessManager
 *
 * \version 0.12.0
 * \author Jochen Ulrich <jochen.ulrich@t-online.de>
 * \copyright © 2018-2023 Jochen Ulrich. Licensed under MIT license (https://opensource.org/licenses/MIT)
 * except for the HttpStatus namespace which is licensed under Creative Commons CC0
 * (http://creativecommons.org/publicdomain/zero/1.0/).
 */
/*
Copyright © 2018-2023 Jochen Ulrich

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of
the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MOCKNETWORKACCESSMANAGER_HPP
#define MOCKNETWORKACCESSMANAGER_HPP

#include <QtGlobal>

#ifdef Q_CC_MSVC
	#pragma warning( push, 0 )
#endif

#include <QAtomicInt>
#include <QtCore>
#include <QtNetwork>

#if QT_VERSION >= QT_VERSION_CHECK( 6,0,0 )
	#if defined( QT_CORE5COMPAT_LIB )
		#include <QtCore5Compat>
	#endif
#endif // Qt >= 6.0.0

#ifdef Q_CC_MSVC
	#pragma warning( pop )
#endif

#include <climits>
#include <cstddef>
#include <memory>
#include <queue>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>
#include <algorithm>

#if ( QT_VERSION < QT_VERSION_CHECK( 6,0,0 ) ) || ( defined( QT_FEATURE_textcodec ) && QT_FEATURE_textcodec == 1 )
	/*! Defined if the QTextCodec is available.
	 *
	 * This is if %Qt version is below 6.0.0 or if the QtCore5Compat library is linked.
	 */
	#define MOCKNETWORKACCESSMANAGER_QT_HAS_TEXTCODEC
#endif

//! \cond QT_POLYFILL
#ifndef Q_NAMESPACE
	#define Q_NAMESPACE
#endif

#ifndef Q_ENUM_NS
	#define Q_ENUM_NS( x )
#endif

#ifndef Q_FALLTHROUGH
	#define Q_FALLTHROUGH()
#endif

#ifndef Q_DECL_DEPRECATED_X
	#define Q_DECL_DEPRECATED_X( x )
#endif

#if QT_VERSION < QT_VERSION_CHECK( 5,14,0 )
namespace std {
template<>
struct hash< QString >
{
	size_t operator()( const QString& string ) const noexcept
	{
		return static_cast< size_t >( qHash( string ) );
	}
};
} // namespace std
#endif // Qt < 5.14.0

//! \endcond

//! \cond MACRO_UTILS
#define MOCKNETWORKACCESS_STRINGIFY( x ) #x
#define MOCKNETWORKACCESS_STRING( x ) MOCKNETWORKACCESS_STRINGIFY( x )
//! \endcond

#if QT_VERSION < QT_VERSION_CHECK( 5,14,0 )
namespace Qt {
using SplitBehaviorFlags = QString::SplitBehavior;
} // namespace Qt
#endif // Qt < 5.14.0

/*! Provides functions and classes to mock network replies in %Qt applications.
 */
namespace MockNetworkAccess {

Q_NAMESPACE

#ifndef MOCKNETWORKACCESS_MAX_PREDEFINED_ATTRIBUTE_ID
	/*! Defines the highest [QNetworkRequest::Attribute] ID that is copied by default when a request was forwarded
	 * to another network access manager.
	 *
	 * Since QNetworkReply::attribute() is not virtual and there is no way of getting all defined attributes of a
	 * QNetworkReply, the network reply returned by the manager needs to know which attributes to copy from a reply of
	 * a forwarded request. By default, it copies attributes up to the ID defined by
	 * MOCKNETWORKACCESS_MAX_PREDEFINED_ATTRIBUTE_ID.
	 *
	 * The default value is 50. This ensures that this version of the MockNetworkAccessManager also works with future
	 * versions of Qt even if they introduce new attributes.
	 *
	 * To override the default value, define it to another value before including MockNetworkAccessManager.hpp:
	 * \code
	 * #define MOCKNETWORKACCESS_MAX_PREDEFINED_ATTRIBUTE_ID 100
	 * #include "MockNetworkAccessManager.hpp"
	 * \endcode
	 *
	 * \note The value should be greater or equal to the actual highest ID of a [QNetworkRequest::Attribute] defined by
	 * %Qt. At the time of writing, this means the value should be at least 28 because this corresponds to
	 * `QNetworkRequest::AutoDeleteReplyOnFinishAttribute` in Qt 5.14.
	 * Additionally, the value must not be higher than 32767 (QNetworkRequest::UserMax). This raises a compile error
	 * because this would trigger undefined behavior.
	 *
	 * \note The mocked network reply will iterate all attributes up to the ID defined by this preprocessor define.
	 * It is also possible to enable copying of single attributes by using
	 * MockNetworkAccess::Manager::registerUserDefinedAttribute(). This is especially sensible for user defined
	 * attributes because they have a rather high ID range.
	 *
	 * \sa MockNetworkAccess::Manager::registerUserDefinedAttribute()
	 * \since 0.11.0
	 *
	 * [QNetworkRequest::Attribute]: http://doc.qt.io/qt-5/qnetworkrequest.html#Attribute-enum
	 */
	#define MOCKNETWORKACCESS_MAX_PREDEFINED_ATTRIBUTE_ID 50
#endif
#if MOCKNETWORKACCESS_MAX_PREDEFINED_ATTRIBUTE_ID > 32767
	#error MOCKNETWORKACCESS_MAX_PREDEFINED_ATTRIBUTE_ID must not be bigger than QNetworkRequest::UserMax \
	       since this would cause undefined behavior.
#endif
#if MOCKNETWORKACCESS_MAX_PREDEFINED_ATTRIBUTE_ID < 28
	#if __GNUC__
		#pragma GCC warning "MOCKNETWORKACCESS_MAX_PREDEFINED_ATTRIBUTE_ID should be at least as big as \
			QNetworkRequest::AutoDeleteReplyOnFinishAttribute. Else it might miss attributes when forwarding a \
			request."
	#elif _MSC_VER
		// clang-format off
		#pragma message( __FILE__ "(" MOCKNETWORKACCESS_STRING(__LINE__) "): warning: " \
			"MOCKNETWORKACCESS_MAX_PREDEFINED_ATTRIBUTE_ID should be at least as big as " \
			"QNetworkRequest::AutoDeleteReplyOnFinishAttribute. Else it might miss attributes when forwarding a " \
			"request."
		) // clang-format on
	#endif
#endif


/*! Returns the logging category used by the library.
 *
 * The name of the category is `MockNetworkAccessManager`.
 * All messages logged by the library use this logging category.
 *
 * \return The QLoggingCategory of the MockNetworkAccessManager library.
 * \since 0.5.0
 */
inline Q_LOGGING_CATEGORY( log, "MockNetworkAccessManager" )

/*! Behavior switches defining different behaviors for the classes of the Manager.
 * \sa page_behaviorFlags
 */
enum BehaviorFlag
{
	/*! Defines that a class behaves as expected according to the documentation and standards
	 * (RFCs etc.). This also means it should behave like most %Qt bugs being fixed
	 * (see \ref page_knownQtBugs for a list of exceptions).
	 * This flag cannot be combined with other BehaviorFlags.
	 * \sa \ref page_knownQtBugs
	 */
	Behavior_Expected = 0,

	/*! Defines that the MockReplies emits an `uploadProgress(0, 0)` signal after the download.
	 * There is QTBUG-44782 in QNetworkReply which causes it to emit an `uploadProgress(0, 0)` signal after
	 * the download of the reply has finished.
	 * \sa https://bugreports.qt.io/browse/QTBUG-44782
	 */
	Behavior_FinalUpload00Signal = 1 << 1,
	/*! Defines that the Manager does not automatically redirect on 308 "Permanent Redirect" responses.
	 * %Qt does not respect the 308 status code for automatic redirection until %Qt 5.9.3 (was fixed with QTBUG-63075).
	 * \sa https://bugreports.qt.io/browse/QTBUG-63075
	 * \since 0.3.0
	 */
	Behavior_NoAutomatic308Redirect = 1 << 2,
	/*! Defines that the Manager follows all redirects with a GET request (except the initial request was a HEAD
	 * request in which case it follows with a HEAD request as well).
	 * %Qt until 5.9.3 followed all redirected requests except HEAD requests with a GET request. QTBUG-63142 fixes this
	 * to not change the request method for 307 and 308 requests.
	 * \sa https://bugreports.qt.io/browse/QTBUG-63142
	 * \since 0.3.0
	 */
	Behavior_RedirectWithGet = 1 << 3,
	/*! Defines that the Manager assumes Latin-1 encoding for username and password for HTTP authentication.
	 * By default, the Manager uses the `charset` parameter of the authentication scheme and defaults to UTF-8 encoding.
	 */
	Behavior_HttpAuthLatin1Encoding = 1 << 4,
	/*! Defines that the Manager rewrites the request verbs OPTIONS and TRACE to GET when following redirects.
	 * [RFC 7231, Section 4.2.1](https://tools.ietf.org/html/rfc7231#section-4.2.1) defines the HTTP verbs OPTIONS and
	 * TRACE as "safe" request methods so it should be fine to use them when automatically following redirects for HTTP
	 * status codes 301 and 302. This behavior defines that the Manager should still redirect with a GET request in that
	 * case.
	 * \note Behavior_RedirectWithGet overrides this flag. So if Behavior_RedirectWithGet is set, this flag is ignored.
	 * \since 0.3.0
	 */
	Behavior_IgnoreSafeRedirectMethods = 1 << 5,

	/*! Defines that the Manager concatenates most raw header values after it followed a redirect.
	 * This is the behavior of QTBUG-61300 which was fixed in %Qt 5.9.3.
	 * \sa https://bugreports.qt.io/browse/QTBUG-61300
	 * \since 0.11.0
	 */
	Behavior_ConcatRawHeadersAfterRedirect = 1 << 6,

	/*! Defines the behavior of %Qt 5.9.3.
	 * \since 0.11.0
	 */
	Behavior_Qt_5_9_3 = Behavior_FinalUpload00Signal      //
	                    | Behavior_HttpAuthLatin1Encoding //
	                    | Behavior_IgnoreSafeRedirectMethods,

	/*! Defines the behavior of %Qt 5.6.
	 */
	Behavior_Qt_5_6_0 = Behavior_Qt_5_9_3                 //
	                    | Behavior_NoAutomatic308Redirect //
	                    | Behavior_RedirectWithGet        //
	                    | Behavior_ConcatRawHeadersAfterRedirect,

	/*! Defines the behavior of %Qt 5.2.
	 * \since 0.3.0
	 */
	Behavior_Qt_5_2_0 = Behavior_Qt_5_6_0
};
/*! QFlags type of \ref BehaviorFlag
 */
Q_DECLARE_FLAGS( BehaviorFlags, BehaviorFlag )
Q_ENUM_NS( BehaviorFlag )

/*! The underlying type of QNetworkRequest::Attribute.
 */
#if __cplusplus >= 201402L
using AttributeIdType = std::underlying_type_t< QNetworkRequest::Attribute >;
#else  // C++ < 14
using AttributeIdType = std::underlying_type< QNetworkRequest::Attribute >::type;
#endif // C++ < 14

// LCOV_EXCL_START
// @sonarcloud-exclude-start
/*! HTTP Status Codes - Qt Variant
 *
 * https://github.com/j-ulrich/http-status-codes-cpp
 *
 * \version 1.5.0
 * \author Jochen Ulrich <jochenulrich@t-online.de>
 * \copyright Licensed under Creative Commons CC0 (http://creativecommons.org/publicdomain/zero/1.0/)
 */
// clang-format off
namespace HttpStatus{
#if(QT_VERSION>=QT_VERSION_CHECK(5,8,0))
Q_NAMESPACE
#endif
enum Code{Continue=100,SwitchingProtocols=101,Processing=102,EarlyHints=103,OK=200,Created=201,Accepted=202,NonAuthoritativeInformation=203,NoContent=204,ResetContent=205,PartialContent=206,MultiStatus=207,AlreadyReported=208,IMUsed=226,MultipleChoices=300,MovedPermanently=301,Found=302,SeeOther=303,NotModified=304,UseProxy=305,TemporaryRedirect=307,PermanentRedirect=308,BadRequest=400,Unauthorized=401,PaymentRequired=402,Forbidden=403,NotFound=404,MethodNotAllowed=405,NotAcceptable=406,ProxyAuthenticationRequired=407,RequestTimeout=408,Conflict=409,Gone=410,LengthRequired=411,PreconditionFailed=412,ContentTooLarge=413,PayloadTooLarge=413,URITooLong=414,UnsupportedMediaType=415,RangeNotSatisfiable=416,ExpectationFailed=417,ImATeapot=418,MisdirectedRequest=421,UnprocessableContent=422,UnprocessableEntity=422,Locked=423,FailedDependency=424,TooEarly=425,UpgradeRequired=426,PreconditionRequired=428,TooManyRequests=429,RequestHeaderFieldsTooLarge=431,UnavailableForLegalReasons=451,InternalServerError=500,NotImplemented=501,BadGateway=502,ServiceUnavailable=503,GatewayTimeout=504,HTTPVersionNotSupported=505,VariantAlsoNegotiates=506,InsufficientStorage=507,LoopDetected=508,NotExtended=510,NetworkAuthenticationRequired=511,xxx_max=1023};
#if(QT_VERSION>=QT_VERSION_CHECK(5,8,0))
Q_ENUM_NS(Code)
#endif
inline bool isInformational(int code){return(code>=100&&code<200);}inline bool isSuccessful(int code){return(code>=200&&code<300);}inline bool isRedirection(int code){return(code>=300&&code<400);}inline bool isClientError(int code){return(code>=400&&code<500);}inline bool isServerError(int code){return(code>=500&&code<600);}inline bool isError(int code){return(code>=400);}inline QString reasonPhrase(int code){switch(code){case 100:return QStringLiteral("Continue");case 101:return QStringLiteral("Switching Protocols");case 102:return QStringLiteral("Processing");case 103:return QStringLiteral("Early Hints");case 200:return QStringLiteral("OK");case 201:return QStringLiteral("Created");case 202:return QStringLiteral("Accepted");case 203:return QStringLiteral("Non-Authoritative Information");case 204:return QStringLiteral("No Content");case 205:return QStringLiteral("Reset Content");case 206:return QStringLiteral("Partial Content");case 207:return QStringLiteral("Multi-Status");case 208:return QStringLiteral("Already Reported");case 226:return QStringLiteral("IM Used");case 300:return QStringLiteral("Multiple Choices");case 301:return QStringLiteral("Moved Permanently");case 302:return QStringLiteral("Found");case 303:return QStringLiteral("See Other");case 304:return QStringLiteral("Not Modified");case 305:return QStringLiteral("Use Proxy");case 307:return QStringLiteral("Temporary Redirect");case 308:return QStringLiteral("Permanent Redirect");case 400:return QStringLiteral("Bad Request");case 401:return QStringLiteral("Unauthorized");case 402:return QStringLiteral("Payment Required");case 403:return QStringLiteral("Forbidden");case 404:return QStringLiteral("Not Found");case 405:return QStringLiteral("Method Not Allowed");case 406:return QStringLiteral("Not Acceptable");case 407:return QStringLiteral("Proxy Authentication Required");case 408:return QStringLiteral("Request Timeout");case 409:return QStringLiteral("Conflict");case 410:return QStringLiteral("Gone");case 411:return QStringLiteral("Length Required");case 412:return QStringLiteral("Precondition Failed");case 413:return QStringLiteral("Content Too Large");case 414:return QStringLiteral("URI Too Long");case 415:return QStringLiteral("Unsupported Media Type");case 416:return QStringLiteral("Range Not Satisfiable");case 417:return QStringLiteral("Expectation Failed");case 418:return QStringLiteral("I'm a teapot");case 421:return QStringLiteral("Misdirected Request");case 422:return QStringLiteral("Unprocessable Content");case 423:return QStringLiteral("Locked");case 424:return QStringLiteral("Failed Dependency");case 425:return QStringLiteral("Too Early");case 426:return QStringLiteral("Upgrade Required");case 428:return QStringLiteral("Precondition Required");case 429:return QStringLiteral("Too Many Requests");case 431:return QStringLiteral("Request Header Fields Too Large");case 451:return QStringLiteral("Unavailable For Legal Reasons");case 500:return QStringLiteral("Internal Server Error");case 501:return QStringLiteral("Not Implemented");case 502:return QStringLiteral("Bad Gateway");case 503:return QStringLiteral("Service Unavailable");case 504:return QStringLiteral("Gateway Timeout");case 505:return QStringLiteral("HTTP Version Not Supported");case 506:return QStringLiteral("Variant Also Negotiates");case 507:return QStringLiteral("Insufficient Storage");case 508:return QStringLiteral("Loop Detected");case 510:return QStringLiteral("Not Extended");case 511:return QStringLiteral("Network Authentication Required");default:return QString();}}inline int networkErrorToStatusCode(QNetworkReply::NetworkError error){switch(error){case QNetworkReply::AuthenticationRequiredError:return Unauthorized;case QNetworkReply::ContentAccessDenied:return Forbidden;case QNetworkReply::ContentNotFoundError:return NotFound;case QNetworkReply::ContentOperationNotPermittedError:return MethodNotAllowed;case QNetworkReply::ProxyAuthenticationRequiredError:return ProxyAuthenticationRequired;case QNetworkReply::NoError:return OK;case QNetworkReply::ProtocolInvalidOperationError:return BadRequest;case QNetworkReply::UnknownContentError:return BadRequest;
#if QT_VERSION>=QT_VERSION_CHECK(5,3,0)
case QNetworkReply::ContentConflictError:return Conflict;case QNetworkReply::ContentGoneError:return Gone;case QNetworkReply::InternalServerError:return InternalServerError;case QNetworkReply::OperationNotImplementedError:return NotImplemented;case QNetworkReply::ServiceUnavailableError:return ServiceUnavailable;case QNetworkReply::UnknownServerError:return InternalServerError;
#endif
default:return-1;}}inline QNetworkReply::NetworkError statusCodeToNetworkError(int code){if(!isError(code))return QNetworkReply::NoError;switch(code){case BadRequest:return QNetworkReply::ProtocolInvalidOperationError;case Unauthorized:return QNetworkReply::AuthenticationRequiredError;case Forbidden:return QNetworkReply::ContentAccessDenied;case NotFound:return QNetworkReply::ContentNotFoundError;case MethodNotAllowed:return QNetworkReply::ContentOperationNotPermittedError;case ProxyAuthenticationRequired:return QNetworkReply::ProxyAuthenticationRequiredError;case ImATeapot:return QNetworkReply::ProtocolInvalidOperationError;
#if QT_VERSION>=QT_VERSION_CHECK(5,3,0)
case Conflict:return QNetworkReply::ContentConflictError;case Gone:return QNetworkReply::ContentGoneError;case InternalServerError:return QNetworkReply::InternalServerError;case NotImplemented:return QNetworkReply::OperationNotImplementedError;case ServiceUnavailable:return QNetworkReply::ServiceUnavailableError;
#endif
default:break;}if(isClientError(code))return QNetworkReply::UnknownContentError;
#if QT_VERSION>=QT_VERSION_CHECK(5,3,0)
if(isServerError(code))return QNetworkReply::UnknownServerError;
#endif
return QNetworkReply::ProtocolFailure;}
} // namespace HttpStatus
// END OF Creative Commons CC0 (http://creativecommons.org/publicdomain/zero/1.0/) LICENSED CODE
// clang-format on
// @sonarcloud-exclude-end
// LCOV_EXCL_STOP

/*! \internal Implementation details
 *
 * \warning The elements in this namespace are **NOT** part of the public API.
 * Anything in this namespace may be changed or removed with any version.
 */
namespace detail {

	/*! \internal
	 * Formats a pointer's address as string.
	 * \param pointer The pointer.
	 * \return A string representing the \p pointer's address.
	 */
	inline QString pointerToQString( const void* pointer )
	{
		// From https://stackoverflow.com/a/16568641/490560
		const int bytesPerHexDigit = 2;
		const int hexBase = 16;
		return QString::fromLatin1( "0x%1" ).arg( reinterpret_cast< quintptr >( pointer ),
		                                          QT_POINTER_SIZE * bytesPerHexDigit,
		                                          hexBase,
		                                          QChar::fromLatin1( '0' ) );
	}

	/* Polyfill for std::as_const() as long as we still support C++ < 17.
	 * qAsConst() is marked deprecated in Qt 6.6 and it doesn't exist in Qt < 5.7.
	 * Therefore, we use our own implementation.
	 */
	template< typename Type >
	constexpr typename std::add_const< Type >::type& asConst( Type& instance ) noexcept
	{
		return instance;
	}
	template< typename Type >
	void asConst( const Type&& ) = delete;

} // namespace detail


/*! Provides helper methods for tasks related to HTTP.
 *
 * \sa https://tools.ietf.org/html/rfc7230
 */
namespace HttpUtils {
	/*! The default port of HTTP requests.
	 */
	const int HttpDefaultPort = 80;

	/*! The default port of HTTPS requests.
	 */
	const int HttpsDefaultPort = 443;

	/*! \return The scheme of the Hypertext Transfer Protocol (HTTP) in lower case characters.
	 */
	inline QString httpScheme()
	{
		const auto httpSchemeString = QStringLiteral( "http" );
		return httpSchemeString;
	}

	/*! \return The scheme of the Hypertext Transfer Protocol Secure (HTTPS) in lower case characters.
	 */
	inline QString httpsScheme()
	{
		const auto httpsSchemeString = QStringLiteral( "https" );
		return httpsSchemeString;
	}

	/*! Checks if a given scheme is the HTTP or HTTPS scheme.
	 * \param scheme The scheme to be checked.
	 * \return \c true if the \p scheme is the HTTP or HTTPS scheme.
	 */
	inline bool isHttpScheme( const QString& scheme )
	{
		return scheme == httpScheme() || scheme == httpsScheme();
	}

	/*! Checks if a URL has a HTTP or HTTPS scheme.
	 * \param url The URL to be checked for an HTTP scheme.
	 * \return \c true if the \p url has an HTTP or HTTPS scheme.
	 */
	inline bool isHttpScheme( const QUrl& url )
	{
		return isHttpScheme( url.scheme() );
	}

	/*! \return The name of the Location header field.
	 */
	inline QByteArray locationHeader()
	{
		const auto locationHeaderKey = QByteArrayLiteral( "Location" );
		return locationHeaderKey;
	}

	/*! \return The name of the Connection header field.
	 */
	inline QByteArray connectionHeader()
	{
		const auto connectionHeaderKey = QByteArrayLiteral( "Connection" );
		return connectionHeaderKey;
	}

	/*! \return The name of the WWW-Authenticate header field.
	 */
	inline QByteArray wwwAuthenticateHeader()
	{
		const auto wwwAuthenticateHeaderKey = QByteArrayLiteral( "WWW-Authenticate" );
		return wwwAuthenticateHeaderKey;
	}

	/*! \return The name of the Proxy-Authenticate header field.
	 */
	inline QByteArray proxyAuthenticateHeader()
	{
		const auto proxyAuthenticateHeaderKey = QByteArrayLiteral( "Proxy-Authenticate" );
		return proxyAuthenticateHeaderKey;
	}

	/*! \return The name of the Authorization header field.
	 */
	inline QByteArray authorizationHeader()
	{
		const auto authorizationHeaderKey = QByteArrayLiteral( "Authorization" );
		return authorizationHeaderKey;
	}

	/*! \return The name of the Proxy-Authorization header field.
	 */
	inline QByteArray proxyAuthorizationHeader()
	{
		const auto proxyAuthorizationHeaderKey = QByteArrayLiteral( "Proxy-Authorization" );
		return proxyAuthorizationHeaderKey;
	}

	/*! \return The regular expression pattern to match tokens according to RFC 7230 3.2.6.
	 */
	inline QString tokenPattern()
	{
		const auto token = QStringLiteral( "(?:[0-9a-zA-Z!#$%&'*+\\-.^_`|~]+)" );
		return token;
	}

	/*! \return The regular expression pattern to match token68 according to RFC 7235 2.1.
	 */
	inline QString token68Pattern()
	{
		const auto token68 = QStringLiteral( "(?:[0-9a-zA-Z\\-._~+\\/]+=*)" );
		return token68;
	}
	/*! \return The regular expression pattern to match successive linear whitespace according to RFC 7230 3.2.3.
	 */
	inline QString lwsPattern()
	{
		const auto lws = QStringLiteral( "(?:[ \t]+)" );
		return lws;
	}
	/*! \return The regular expression pattern to match obsolete line folding (obs-fold) according to RFC 7230 3.2.4.
	 */
	inline QString obsFoldPattern()
	{
		const auto obsFold = QStringLiteral( "(?:\r\n[ \t])" );
		return obsFold;
	}
	/*! Returns a version of a string with linear whitespace according to RFC 7230 3.2.3 removed from the
	 * beginning and end of the string.
	 *
	 * \param string The string whose leading and trailing linear whitespace should be removed.
	 * \return A copy of \p string with all horizontal tabs and spaces removed from the beginning and end of the
	 * string.
	 */
	inline QString trimmed( const QString& string )
	{
		const QRegularExpression leadingLwsRegEx( QStringLiteral( "^" ) + lwsPattern() + QStringLiteral( "+" ) );
		const QRegularExpression trailingLwsRegEx( lwsPattern() + QStringLiteral( "+$" ) );

		QString trimmed( string );

		const QRegularExpressionMatch leadingMatch = leadingLwsRegEx.match( trimmed );
		if ( leadingMatch.hasMatch() )
			trimmed.remove( 0, leadingMatch.capturedLength( 0 ) );

		const QRegularExpressionMatch trailingMatch = trailingLwsRegEx.match( trimmed );
		if ( trailingMatch.hasMatch() )
			trimmed.remove( trailingMatch.capturedStart( 0 ), trailingMatch.capturedLength( 0 ) );

		return trimmed;
	}
	/*! Returns a version of a string with obsolete line folding replaced with a space and whitespace trimmed,
	 * both according to RFC 7230.
	 *
	 * \param string The string which should be trimmed and whose obs-folding should be removed.
	 * \return A copy of \p string with all obsolete line foldings (RFC 7230 3.2.4) replaced with a space
	 * and afterwards, trimmed using trimmed().
	 *
	 * \sa trimmed()
	 */
	inline QString whiteSpaceCleaned( const QString& string )
	{
		const QRegularExpression obsFoldRegEx( obsFoldPattern() );
		QString cleaned( string );
		cleaned.replace( obsFoldRegEx, QLatin1String( " " ) );
		return trimmed( cleaned );
	}

	/*! Checks if a given string is a token according to RFC 7230 3.2.6.
	 *
	 * \param string The string to be checked to be a token.
	 * \return \c true if \p string is a valid token or \c false otherwise.
	 */
	inline bool isValidToken( const QString& string )
	{
		const QRegularExpression tokenRegEx( QStringLiteral( "^" ) + tokenPattern() + QStringLiteral( "$" ) );
		return tokenRegEx.match( string ).hasMatch();
	}

	/*! Checks if a character is a visible (printable) US ASCII character.
	 *
	 * @param character The character to be checked.
	 * @return \c true if \p character is a printable US ASCII character.
	 */
	inline bool isVCHAR( const char character )
	{
		const char FirstVCHAR = '\x21';
		const char LastVCHAR = '\x7E';

		return character >= FirstVCHAR && character <= LastVCHAR;
	}

	/*! Checks if a character is an "obs-text" character according to RFC 7230 3.2.6.
	 *
	 * @param character The character to be checked.
	 * @return \c true if \p character falls into the "obs-text" character range.
	 */
	inline bool isObsTextChar( const char character )
	{
#if CHAR_MIN < 0
		// char is signed so all obs-text characters are negative
		return character < 0;
#else
		const char FirstObsTextChar = '\x80';

		/* LastObsTextChar would be 0xFF which is the maximum value of char
		 * so there is no need to check if character is smaller.
		 */

		return character >= FirstObsTextChar;
#endif
	}

	/*! Checks if a character is legal to occur in a header field according to RFC 7230 3.2.6.
	 *
	 * \param character The character to be checked.
	 * \return \c true if \p character is an allowed character for a header field value.
	 */
	inline bool isLegalHeaderCharacter( const char character )
	{
		return ( character == QChar::Tabulation || character == QChar::Space || isVCHAR( character )
		         || isObsTextChar( character ) );
	}

	/*! Checks if a string is a valid quoted-string according to RFC 7230 3.2.6.
	 *
	 * \param string The string to be tested. \p string is expected to *not* contain obsolete line folding (obs-fold).
	 * Use whiteSpaceCleaned() to ensure this.
	 * \return \c true if the \p string is a valid quoted-string.
	 */
	inline bool isValidQuotedString( const QString& string )
	{
		// String needs to contain at least the quotes
		const QString::size_type minimumStringSize = 2;
		if ( string.size() < minimumStringSize )
			return false;

		// First character must be a quote
		if ( string.at( 0 ).toLatin1() != '"' )
			return false;

		unsigned int backslashCount = 0;
		const int backslashEscapeLength = 2;
		for ( QString::size_type i = 1, stringContentEnd = string.size() - 1; i < stringContentEnd; ++i )
		{
			// Non-Latin-1 characters will be 0
			const auto c = string.at( i ).toLatin1();

			// String must not contain illegal characters
			if ( Q_UNLIKELY( ! isLegalHeaderCharacter( c ) ) )
				return false;

			if ( c == '\\' )
				++backslashCount;
			else
			{
				// Other quotes and obs-text must be escaped
				if ( ( c == '"' || isObsTextChar( c ) ) && ( backslashCount % backslashEscapeLength ) == 0 )
					return false;

				backslashCount = 0;
			}
		}

		// Last character must be a quote and it must not be escaped
		if ( string.at( string.size() - 1 ).toLatin1() != '"' || ( backslashCount % backslashEscapeLength ) != 0 )
			return false;

		return true;
	}

	/*! Converts a quoted-string according to RFC 7230 3.2.6 to it's unquoted version.
	 *
	 * \param quotedString The quoted string to be converted to "plain" text.
	 * \return A copy of \p quotedString with all quoted-pairs converted to the second character of the pair and the
	 * leading and trailing double quotes removed. If \p quotedString is not a valid quoted-string, a null
	 * QString() is returned.
	 */
	inline QString unquoteString( const QString& quotedString )
	{
		if ( ! isValidQuotedString( quotedString ) )
			return QString();

		QString unquotedString( quotedString.mid( 1, quotedString.size() - 2 ) );

		const QRegularExpression quotedPairRegEx( QStringLiteral( "\\\\." ) );
		QStack< QRegularExpressionMatch > quotedPairMatches;
		QRegularExpressionMatchIterator quotedPairIter = quotedPairRegEx.globalMatch( unquotedString );
		while ( quotedPairIter.hasNext() )
			quotedPairMatches.push( quotedPairIter.next() );

		while ( ! quotedPairMatches.isEmpty() )
		{
			const QRegularExpressionMatch match = quotedPairMatches.pop();
			unquotedString.remove( match.capturedStart( 0 ), 1 );
		}

		return unquotedString;
	}

	/*! Converts a string to it's quoted version according to RFC 7230 3.2.6.
	 *
	 * \param unquotedString The "plain" text to be converted to a quoted-string.
	 * \return A copy of \p unquotedString surrounded with double quotes and all double quotes, backslashes
	 * and obs-text characters escaped. If the \p unquotedString contains any characters that are not allowed
	 * in a header field value, a null QString() is returned.
	 */
	inline QString quoteString( const QString& unquotedString )
	{
		QString escapedString;

		for ( auto&& qChar : unquotedString )
		{
			// Non-Latin-1 characters will be 0
			const auto c = qChar.toLatin1();

			if ( Q_UNLIKELY( ! isLegalHeaderCharacter( c ) ) )
				return QString();

			if ( c == '"' || c == '\\' || isObsTextChar( c ) )
				escapedString += QChar::fromLatin1( '\\' );
			escapedString += QChar::fromLatin1( c );
		}

		return QStringLiteral( "\"" ) + escapedString + QStringLiteral( "\"" );
	}

	/*! \internal Implementation details
	 */
	namespace detail {
		class CommaSeparatedListParser
		{
		public:
			CommaSeparatedListParser()
			    : m_inString( false )
			    , m_escaped( false )
			{
			}

			QStringList parse( const QString& commaSeparatedList )
			{
				QString::const_iterator iter = commaSeparatedList.cbegin();
				const QString::const_iterator end = commaSeparatedList.cend();
				for ( ; iter != end; ++iter )
				{
					processCharacter( *iter );
				}

				if ( ! checkStateAfterParsing() )
					return QStringList();

				finalizeNextEntry();

				return m_split;
			}

		private:
			void processCharacter( QChar character )
			{
				if ( m_inString )
					processCharacterInString( character );
				else
					processCharacterOutsideString( character );
			}

			void processCharacterInString( QChar character )
			{
				if ( character == QChar::fromLatin1( '\\' ) )
					m_escaped = ! m_escaped;
				else
				{
					if ( character == QChar::fromLatin1( '"' ) && ! m_escaped )
						m_inString = false;
					m_escaped = false;
				}
				m_nextEntry += character;
			}

			void processCharacterOutsideString( QChar character )
			{
				if ( character == QChar::fromLatin1( ',' ) )
				{
					finalizeNextEntry();
				}
				else
				{
					if ( character == QChar::fromLatin1( '"' ) )
						m_inString = true;
					m_nextEntry += character;
				}
			}

			void finalizeNextEntry()
			{
				const QString trimmedEntry = trimmed( m_nextEntry );
				if ( ! trimmedEntry.isEmpty() )
					m_split << trimmedEntry;
				m_nextEntry.clear();
			}

			bool checkStateAfterParsing() const
			{
				return ! m_inString;
			}

		private:
			bool m_inString;
			bool m_escaped;
			QString m_nextEntry;
			QStringList m_split;
		};
	} // namespace detail

	/*! Splits a string containing a comma-separated list according to RFC 7230 section 7.
	 *
	 * \param commaSeparatedList A string containing a comma-separated list. The list can contain
	 * quoted strings and commas within quoted strings are not treated as list separators.
	 * \return QStringList consisting of the elements of \p commaSeparatedList.
	 * Empty elements in \p commaSeparatedList are omitted.
	 */
	inline QStringList splitCommaSeparatedList( const QString& commaSeparatedList )
	{
		detail::CommaSeparatedListParser parser;
		return parser.parse( commaSeparatedList );
	}


	/*! Namespace for HTTP authentication related classes.
	 *
	 * \sa https://tools.ietf.org/html/rfc7235
	 */
	namespace Authentication {
		/*! Returns the authentication scope of a URL according to RFC 7617 2.2.
		 *
		 * \param url The URL whose authentication scope should be returned.
		 * \return A URL which has the same scheme, host, port and path up to the last slash
		 * as \p url.
		 */
		inline QUrl authenticationScopeForUrl( const QUrl& url )
		{
			QUrl authScope;
			authScope.setScheme( url.scheme() );
			authScope.setHost( url.host() );
			authScope.setPort( url.port() );
			const QFileInfo urlPath( url.path( QUrl::FullyEncoded ) );
			QString path = urlPath.path(); // Remove the part after the last slash using QFileInfo::path()
			if ( path.isEmpty() || path == QLatin1String( "." ) )
				path = QLatin1String( "/" );
			else if ( ! path.endsWith( QChar::fromLatin1( '/' ) ) )
				path += QChar::fromLatin1( '/' );
			authScope.setPath( path );
			return authScope;
		}

		/*! \internal Implementation details
		 */
		namespace detail {

			inline QByteArray authorizationHeaderKey()
			{
				const auto authHeader = QByteArrayLiteral( "Authorization" );
				return authHeader;
			}

			inline QString authParamPattern()
			{
				const auto authParam = QStringLiteral( "(?:(?<authParamName>" ) + HttpUtils::tokenPattern()
				                       + QStringLiteral( ")" )                                                   //
				                       + HttpUtils::lwsPattern() + QStringLiteral( "?" )                         //
				                       + QStringLiteral( "=" ) + HttpUtils::lwsPattern() + QStringLiteral( "?" ) //
				                       + QStringLiteral( "(?<authParamValue>" ) + HttpUtils::tokenPattern()      //
				                       + QStringLiteral( "|\".*\"))" );
				return authParam;
			}

		} // namespace detail


		/*! Represents an HTTP authentication challenge according to RFC 7235.
		 */
		class Challenge
		{
		public:
			/*! QSharedPointer to a Challenge object.
			 */
			using Ptr = QSharedPointer< Challenge >;

			/*! Defines the supported HTTP authentication schemes.
			 */
			enum AuthenticationScheme
			{
				/* WARNING: The numerical value defines the preference when multiple
				 * challenges are provided by the server.
				 * The lower the numerical value, the lesser the preference.
				 * So give stronger methods higher values.
				 * See strengthGreater()
				 */
				BasicAuthenticationScheme = 100, //!< HTTP Basic authentication according to RFC 7617
				UnknownAuthenticationScheme = -1 //!< Unknown authentication scheme
			};

			/*! Creates an invalid authentication challenge.
			 *
			 * Sets the behaviorFlags() to Behavior_Expected.
			 */
			Challenge()
			{
				setBehaviorFlags( Behavior_Expected );
			}
			/*! Enforces a virtual destructor.
			 */
			virtual ~Challenge() {}

			/*! \return The authentication scheme of this Challenge.
			 */
			virtual AuthenticationScheme scheme() const = 0;

			/*! Checks if the Challenge is valid, meaning it contains all
			 * parameters required for the authentication scheme.
			 *
			 * \return \c true if the Challenge is valid.
			 */
			virtual bool isValid() const = 0;
			/*! \return The parameters of the Challenge as a QVariantMap.
			 */
			virtual QVariantMap parameters() const = 0;
			/*! \return The value of an Authenticate header representing this Challenge.
			 */
			virtual QByteArray authenticateHeader() const = 0;
			/*! Compares the cryptographic strength of this Challenge with another
			 * Challenge.
			 *
			 * \param other The Challenge to compare against.
			 * \return \c true if this Challenge is considered cryptographically
			 * stronger than \p other. If they are equal or if \p other is stronger,
			 * \c false is returned.
			 */
			virtual bool strengthGreater( const Challenge::Ptr& other )
			{
				return this->scheme() > other->scheme();
			}

			/*! Tunes the behavior of this Challenge.
			 *
			 * \param behaviorFlags Combination of BehaviorFlags to define some details of this Challenge's behavior.
			 * \note Only certain BehaviorFlags have an effect on a Challenge.
			 * \sa BehaviorFlag
			 */
			void setBehaviorFlags( BehaviorFlags behaviorFlags )
			{
				m_behaviorFlags = behaviorFlags;
			}

			/*! \return The BehaviorFlags currently active for this Challenge.
			 */
			BehaviorFlags behaviorFlags() const
			{
				return m_behaviorFlags;
			}

			/*! \return The realm of this Challenge according to RFC 7235 2.2.
			 */
			QString realm() const
			{
				return m_realm;
			}

			/*! \return The name of the realm parameter. Also used as key in QVariantMaps.
			 */
			static QString realmKey()
			{
				return QStringLiteral( "realm" );
			}

			/*! Adds an authorization header for this Challenge to a given request.
			 *
			 * \param request The QNetworkRequest to which the authorization header will be added.
			 * \param operation The HTTP verb.
			 * \param body The message body of the request.
			 * \param authenticator The QAuthenticator providing the credentials to be used for the
			 * authorization.
			 */
			void addAuthorization( QNetworkRequest& request,
			                       QNetworkAccessManager::Operation operation,
			                       const QByteArray& body,
			                       const QAuthenticator& authenticator )
			{
				const QByteArray authHeaderValue = authorizationHeaderValue( request, operation, body, authenticator );
				request.setRawHeader( detail::authorizationHeaderKey(), authHeaderValue );
			}

			/*! Verifies if a given request contains a valid authorization for this Challenge.
			 *
			 * \param request The request which requests authorization.
			 * \param authenticator The QAuthenticator providing a set of valid credentials.
			 * \return \c true if the \p request contains a valid authorization header matching
			 * this Challenge and the credentials provided by the \p authenticator.
			 * Note that for certain authentication schemes, this method might always return \c false if this Challenge
			 * is invalid (see isValid()).
			 */
			virtual bool verifyAuthorization( const QNetworkRequest& request, const QAuthenticator& authenticator ) = 0;

			/*! Implements a "lesser" comparison based on the cryptographic strength of a Challenge.
			 */
			struct StrengthCompare
			{
				/*! Implements the lesser comparison.
				 *
				 * \param left The left-hand side Challenge of the comparison.
				 * \param right The right-hand side Challenge of the comparison.
				 * \return \c true if \p left < \p right regarding the strength of the algorithm
				 * used by the challenges. Otherwise \c false.
				 */
				bool operator()( const Challenge::Ptr& left, const Challenge::Ptr& right ) const
				{
					return right->strengthGreater( left );
				}
			};

		protected:
			/*! Generates a new authorization header value for this Challenge.
			 *
			 * \note This method is non-const because an authentication scheme might need
			 * to remember parameters from the authorizations it gave (like the \c cnonce in the Digest scheme).
			 *
			 * \param request The request for with the authorization header should be generated.
			 * \param operation The HTTP verb of the request.
			 * \param body The message body of the request.
			 * \param authenticator The QAuthenticator providing the credentials to be used to generate the
			 * authorization header.
			 * \return The value of the Authorization header to request authorization for this Challenge using the
			 * credentials provided by the \p authenticator.
			 *
			 * \sa addAuthorization()
			 */
			virtual QByteArray authorizationHeaderValue( const QNetworkRequest& request,
			                                             QNetworkAccessManager::Operation operation,
			                                             const QByteArray& body,
			                                             const QAuthenticator& authenticator ) = 0;

			/*! Splits a list of authentication parameters according to RFC 7235 2.1. into a QVariantMap.
			 *
			 * \param authParams The list of name=value strings.
			 * \param[out] paramsValid If not \c NULL, the value of this boolean will be set to \c false if one of the
			 * parameters in \p authParams was malformed or to \c true otherwise. If \p paramsValid is \c NULL, it is
			 * ignored.
			 * \return A QVariantMap mapping the names of the authentication parameters to their values.
			 * The names of the authentication parameters are converted to lower case.
			 * The values are *not* unquoted in case they are quoted strings.
			 */
			static QVariantMap stringParamListToMap( const QStringList& authParams, bool* paramsValid = Q_NULLPTR )
			{
				QVariantMap result;

				QStringList::const_iterator paramIter = authParams.cbegin();
				const QStringList::const_iterator paramsEnd = authParams.cend();
				const QRegularExpression authParamRegEx( detail::authParamPattern() );

				for ( ; paramIter != paramsEnd; ++paramIter )
				{
					const QRegularExpressionMatch authParamMatch = authParamRegEx.match( *paramIter );
					if ( ! authParamMatch.hasMatch() )
					{
						qCWarning( log ) << "Invalid authentication header: malformed auth-param:" << *paramIter;
						if ( paramsValid )
							*paramsValid = false;
						return QVariantMap();
					}
					const QString authParamName = authParamMatch.captured( QStringLiteral( "authParamName" ) ).toLower();
					const QString authParamValue = authParamMatch.captured( QStringLiteral( "authParamValue" ) );

					if ( result.contains( authParamName ) )
						qCWarning( log ) << "Invalid authentication header: auth-param occurred multiple times:"
						                 << authParamName;

					result.insert( authParamName, authParamValue );
				}

				if ( paramsValid )
					*paramsValid = true;
				return result;
			}

			/*! Sets the realm of this Challenge.
			 *
			 * The base class does not use the realm. It just provides the property for convenience.
			 * So derived classes are free to use the realm as they need to.
			 *
			 * \param realm The realm.
			 * \sa realm()
			 */
			void setRealm( const QString& realm )
			{
				m_realm = realm;
			}

		private:
			QString m_realm;
			BehaviorFlags m_behaviorFlags;
		};

		/*! HTTP Basic authentication scheme according to RFC 7617.
		 *
		 * \sa https://tools.ietf.org/html/rfc7617
		 */
		class Basic : public Challenge
		{
		public:
			/*! Creates a Basic authentication Challenge with parameters as a QStringList.
			 *
			 * \param authParams The parameters of the challenge as a list of name=value strings.
			 */
			explicit Basic( const QStringList& authParams )
			    : Challenge()
			{
				bool paramsValid = false;
				const QVariantMap authParamsMap = stringParamListToMap( authParams, &paramsValid );
				if ( paramsValid )
					readParameters( authParamsMap );
			}

			/*! Creates a Basic authentication Challenge with parameters a QVariantMap.
			 *
			 * \param authParams The parameters of the challenge as a map.
			 */
			explicit Basic( const QVariantMap& authParams )
			    : Challenge()
			{
				readParameters( authParams );
			}

			/*! Creates a Basic authentication Challenge with the given realm.
			 *
			 * \param realm The realm.
			 */
			explicit Basic( const QString& realm )
			    : Challenge()
			{
				QVariantMap params;
				params.insert( realmKey(), realm );
				readParameters( params );
			}


			/*! \return Challenge::BasicAuthenticationScheme.
			 */
			AuthenticationScheme scheme() const override
			{
				return BasicAuthenticationScheme;
			}

			/*! \return The identifier string of the Basic authentication scheme.
			 */
			static QByteArray schemeString()
			{
				return "Basic";
			}

			/*! \return The name of the charset parameter. Also used as key in QVariantMaps.
			 */
			static QString charsetKey()
			{
				return QStringLiteral( "charset" );
			}

			/*! \return \c true if the realm parameter is defined. Note that the realm can still
			 * be empty (`""`).
			 */
			bool isValid() const override
			{
				return ! realm().isNull();
			}

			/*! \return A map containing the realm and charset parameters (if given).
			 * \sa realmKey(), charsetKey().
			 */
			QVariantMap parameters() const override
			{
				QVariantMap params;
				params[ realmKey() ] = realm();
				if ( ! m_charset.isEmpty() )
					params[ charsetKey() ] = m_charset;
				return params;
			}
			/*! \copydoc Challenge::authenticateHeader()
			 */
			QByteArray authenticateHeader() const override
			{
				if ( ! isValid() )
					return QByteArray();

				QByteArray result = schemeString() + " " + realmKey().toLatin1() + "="
				                    + quoteString( realm() ).toLatin1();
				if ( ! m_charset.isEmpty() )
					result += ", " + charsetKey().toLatin1() + "=" + quoteString( m_charset ).toLatin1();
				return result;
			}

			/*! \copydoc Challenge::verifyAuthorization(const QNetworkRequest&, const QAuthenticator&)
			 */
			bool verifyAuthorization( const QNetworkRequest& request, const QAuthenticator& authenticator ) override
			{
				/* Since the authorization header of the Basic scheme is very simple, we can simply compare
				 * the textual representations.
				 * Additionally, we can verify the authorization even if this challenge is invalid.
				 */
				const QByteArray reqAuth = request.rawHeader( detail::authorizationHeaderKey() );
				const QByteArray challengeAuth = this->authorizationHeaderValue( QNetworkRequest(),
				                                                                 QNetworkAccessManager::GetOperation,
				                                                                 QByteArray(),
				                                                                 authenticator );
				return reqAuth == challengeAuth;
			}


		protected:
			/*! \copydoc Challenge::authorizationHeaderValue()
			 */
			QByteArray authorizationHeaderValue( const QNetworkRequest& request,
			                                     QNetworkAccessManager::Operation operation,
			                                     const QByteArray& body,
			                                     const QAuthenticator& authenticator ) override
			{
				Q_UNUSED( request )
				Q_UNUSED( operation )
				Q_UNUSED( body )

				QByteArray userName;
				QByteArray password;
				if ( behaviorFlags().testFlag( Behavior_HttpAuthLatin1Encoding ) )
				{
					userName = authenticator.user().toLatin1();
					password = authenticator.password().toLatin1();
				}
				else
				{
					/* No need to check m_charset since UTF-8 is the only allowed encoding at the moment and
					 * we use UTF-8 by default anyway (so even if charset was not specified)
					 */
					userName = authenticator.user().normalized( QString::NormalizationForm_C ).toUtf8();
					password = authenticator.password().normalized( QString::NormalizationForm_C ).toUtf8();
				}

				return schemeString() + " " + ( userName + ":" + password ).toBase64();
			}

		private:
			void readParameters( const QVariantMap& params )
			{
				if ( ! params.contains( realmKey() ) )
				{
					setRealm( QString() );
					qCWarning( log ) << "Invalid authentication header: Missing required parameter: \"realm\"";
					return;
				}

				// Realm
				const QString realmValue = params.value( realmKey() ).toString();
				const QString realm = HttpUtils::isValidToken( realmValue ) ? realmValue
				                                                            : HttpUtils::unquoteString( realmValue );
				if ( realm.isNull() )
				{
					qCWarning( log ) << "Invalid authentication header: Missing value for parameter: \"realm\"";
					return;
				}
				setRealm( realm );

				// Charset
				if ( params.contains( charsetKey() ) )
				{
					const QString charsetValue = params.value( charsetKey() ).toString();
					const QString charset = ( HttpUtils::isValidToken( charsetValue )
					                              ? charsetValue
					                              : HttpUtils::unquoteString( charsetValue ) )
					                            .toLower();
					m_charset = charset;
				}
			}

			QString m_charset;
		};

		/*! \internal Implementation details
		 */
		namespace detail {
			inline Challenge::Ptr parseAuthenticateChallenge( const QStringList& challengeParts, const QUrl& )
			{
				const auto& challengeStart = challengeParts.at( 0 );
				const auto schemeSeparatorIndex = challengeStart.indexOf( QChar::fromLatin1( ' ' ) );
				const auto authSchemeLower = HttpUtils::trimmed( challengeStart.left( schemeSeparatorIndex ) ).toLower();
				const auto firstAuthParam = ( schemeSeparatorIndex > 0 )
				                                ? HttpUtils::trimmed( challengeStart.mid( schemeSeparatorIndex + 1 ) )
				                                : QString();

				// Get the first parameter of the challenge
				QStringList authParams;
				if ( ! firstAuthParam.isEmpty() )
					authParams << firstAuthParam;
				// Append further parameters of the challenge
				if ( challengeParts.size() > 1 )
					authParams << challengeParts.mid( 1 );

				const QString basicAuthSchemeLower = QString::fromLatin1( Basic::schemeString() ).toLower();
				if ( authSchemeLower == basicAuthSchemeLower )
					return Challenge::Ptr( new Basic( authParams ) );

				qCWarning( log ) << "Unsupported authentication scheme:" << authSchemeLower;
				return Challenge::Ptr();
			}

			inline QVector< QStringList > splitAuthenticateHeaderIntoChallengeParts( const QString& headerValue )
			{
				QVector< QStringList > result;

				const QStringList headerSplit = HttpUtils::splitCommaSeparatedList( headerValue );

				const QRegularExpression challengeStartRegEx(
				    QStringLiteral( "^" ) + HttpUtils::tokenPattern() + QStringLiteral( "(?:" )
				    + HttpUtils::lwsPattern() + QStringLiteral( "(?:" ) + HttpUtils::token68Pattern()
				    + QStringLiteral( "|" ) + detail::authParamPattern() + QStringLiteral( "))?" ) );

				QVector< QPair< QString::size_type, QString::size_type > > challengeIndexes;
				auto challengeStartIndex = headerSplit.indexOf( challengeStartRegEx );
				if ( challengeStartIndex < 0 )
				{
					qCWarning( log ) << "Invalid authentication header: expected start of authentication challenge";
					return result;
				}
				while ( challengeStartIndex != -1 )
				{
					const auto nextChallengeStartIndex = headerSplit.indexOf( challengeStartRegEx,
					                                                          challengeStartIndex + 1 );
					challengeIndexes << ::qMakePair( challengeStartIndex, nextChallengeStartIndex );
					challengeStartIndex = nextChallengeStartIndex;
				}

				auto challengeIndexIter = challengeIndexes.cbegin();
				const auto challengeIndexesEnd = challengeIndexes.cend();

				for ( ; challengeIndexIter != challengeIndexesEnd; ++challengeIndexIter )
				{
					const auto challengePartCount = ( challengeIndexIter->second == -1 )
					                                    ? ( headerSplit.size() - challengeIndexIter->first )
					                                    : ( challengeIndexIter->second - challengeIndexIter->first );
					const QStringList challengeParts = headerSplit.mid( challengeIndexIter->first, challengePartCount );

					result << challengeParts;
				}

				return result;
			}

			inline QVector< Challenge::Ptr > parseAuthenticateHeader( const QString& headerValue,
			                                                          const QUrl& requestingUrl )
			{
				QVector< Challenge::Ptr > result;

				const QVector< QStringList > challenges = splitAuthenticateHeaderIntoChallengeParts( headerValue );

				QVector< QStringList >::const_iterator challengeIter = challenges.cbegin();
				const QVector< QStringList >::const_iterator challengesEnd = challenges.cend();

				for ( ; challengeIter != challengesEnd; ++challengeIter )
				{
					const Challenge::Ptr authChallenge = parseAuthenticateChallenge( *challengeIter, requestingUrl );

					if ( authChallenge && authChallenge->isValid() )
						result << authChallenge;
				}

				return result;
			}


			inline QVector< Challenge::Ptr > parseAuthenticateHeaders( const QNetworkReply* reply )
			{
				const auto wwwAuthenticateHeaderLower = wwwAuthenticateHeader().toLower();
				QVector< Challenge::Ptr > authChallenges;
				const auto requestingUrl = reply->url();

				const auto rawHeaderList = reply->rawHeaderList();
				for ( auto&& header : rawHeaderList )
				{
					if ( header.toLower() == wwwAuthenticateHeaderLower )
					{
						const auto headerValue = HttpUtils::whiteSpaceCleaned(
						    QString::fromLatin1( reply->rawHeader( header ) ) );
						if ( headerValue.isEmpty() )
							continue;

						authChallenges << parseAuthenticateHeader( headerValue, requestingUrl );
					}
				}
				return authChallenges;
			}

		} // namespace detail

		/*! Extracts all authentication challenges from a QNetworkReply.
		 *
		 * \param reply The reply object potentially containing authentication challenges.
		 * \return A vector of Challenge::Ptrs. The vector can be empty if \p reply did not
		 * contain any authentication challenges.
		 */
		inline QVector< Challenge::Ptr > getAuthenticationChallenges( const QNetworkReply* reply )
		{
			const auto statusCode = static_cast< HttpStatus::Code >(
			    reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt() );
			switch ( statusCode )
			{
				case HttpStatus::Unauthorized:
					return detail::parseAuthenticateHeaders( reply );

				case HttpStatus::ProxyAuthenticationRequired:
					// TODO: Implement proxy authentication
					qCWarning( log ) << "Proxy authentication is not supported at the moment";
					break;

				// LCOV_EXCL_START
				default:
					Q_ASSERT_X( false, Q_FUNC_INFO, "Trying to authenticate request which doesn't require authentication" );
					break;
					// LCOV_EXCL_STOP
			}

			return QVector< Challenge::Ptr >();
		}

	} // namespace Authentication

} // namespace HttpUtils

/*! Provides helper methods for tasks related to FTP.
 *
 * \since 0.6.0
 */
namespace FtpUtils {

	/*! The default port of FTP requests.
	 */
	const int FtpDefaultPort = 21;

	/*! \return The scheme of the File Transfer Protocol (FTP) in lower case characters.
	 * \since 0.6.0
	 */
	inline QString ftpScheme()
	{
		const auto ftpSchemeString = QStringLiteral( "ftp" );
		return ftpSchemeString;
	}

	/*! \return The scheme of the File Transfer Protocol over SSL (FTPS) in lower case characters.
	 * \since 0.6.0
	 */
	inline QString ftpsScheme()
	{
		const auto ftpsSchemeString = QStringLiteral( "ftps" );
		return ftpsSchemeString;
	}

} // namespace FtpUtils


/*! Provides helper methods for tasks related to data: URLs.
 *
 * \since 0.9.0
 */
namespace DataUrlUtils {
	/*! \return The scheme of data: URLs in lower case characters.
	 * \since 0.9.0
	 */
	inline QString dataScheme()
	{
		const auto dataSchemeString = QStringLiteral( "data" );
		return dataSchemeString;
	}
} // namespace DataUrlUtils


/*! Provides helper methods for tasks related to file: and qrc: URLs.
 *
 * \since 0.9.0
 */
namespace FileUtils {
	/*! \return The scheme of file: URLs in lower case characters.
	 * \since 0.9.0
	 */
	inline QString fileScheme()
	{
		const auto fileSchemeString = QStringLiteral( "file" );
		return fileSchemeString;
	}

	/*! \return The scheme of qrc: URLs in lower case characters.
	 * \since 0.9.0
	 */
	inline QString qrcScheme()
	{
		const auto qrcSchemeString = QStringLiteral( "qrc" );
		return qrcSchemeString;
	}

#if defined( Q_OS_ANDROID )
	inline QString assetsScheme()
	{
		const auto assetsSchemeString = QStringLiteral( "assets" );
		return assetsSchemeString;
	}
#endif

	/*! Checks if a scheme behaves like the file scheme.
	 * \param scheme The scheme to be checked to behave like the file scheme.
	 * \return \c true if the \p url has a `file:`, `qrc:` or on Android `assets:` scheme. \c false otherwise.
	 */
	inline bool isFileLikeScheme( const QString& scheme )
	{
#if defined( Q_OS_ANDROID )
		if ( scheme == assetsScheme() )
			return true;
#endif
		return scheme == fileScheme() || scheme == qrcScheme();
	}

	/*! Checks if a URL has a file-like scheme.
	 * \param url The URL to be checked for a file-like scheme.
	 * \return \c true if the \p url has a file: or qrc: scheme. \c false otherwise.
	 */
	inline bool isFileLikeScheme( const QUrl& url )
	{
		return isFileLikeScheme( url.scheme() );
	}

} // namespace FileUtils

/** Contains helper functions for QNetworkReplies.
 *
 * \since 0.11.0
 */
namespace NetworkReplyUtils {
	/*! Returns the URL of the HTTP Location header field of a given QNetworkReply.
	 * This is a workaround for QTBUG-4106 which prevents that the QNetworkReply::header() method returns a valid
	 * QUrl for relative redirection URLs.
	 * \param reply The QNetworkReply for which the Location header should be returned.
	 * \return The value of the Location header field as a QUrl.
	 * \sa https://bugreports.qt.io/browse/QTBUG-41061
	 * \since 0.4.0
	 * \since 0.11.0 moved to namespace NetworkReplyUtils from MockReply
	 */
	inline QUrl locationHeader( const QNetworkReply* reply )
	{
		const auto rawHeader = reply->rawHeader( HttpUtils::locationHeader() );
		if ( rawHeader.isEmpty() )
			return QUrl();
		else
			return QUrl::fromEncoded( rawHeader, QUrl::StrictMode );
	}

	/*! Checks if a given reply indicates a redirect that can be followed automatically.
	 * \param reply The QNetworkReply to be checked for a redirect.
	 * \param behaviorFlags The BehaviorFlags to be considered.
	 * \return \c true if the \p reply's HTTP status code is valid and indicates a redirect that can be followed
	 * automatically.
	 * \since 0.11.0
	 * \sa \ref BehaviorFlag
	 */
	inline bool isRedirectToBeFollowed( const QNetworkReply* reply, BehaviorFlags behaviorFlags )
	{
		const auto statusCodeAttr = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
		if ( ! statusCodeAttr.isValid() )
			return false;

		switch ( statusCodeAttr.toInt() )
		{
			case HttpStatus::MovedPermanently:  // 301
			case HttpStatus::Found:             // 302
			case HttpStatus::SeeOther:          // 303
			case HttpStatus::UseProxy:          // 305
			case HttpStatus::TemporaryRedirect: // 307
				return true;
			case HttpStatus::PermanentRedirect: // 308
				if ( behaviorFlags.testFlag( Behavior_NoAutomatic308Redirect ) )
					return false; // Qt doesn't recognize 308 for automatic redirection
				else
					return true;
			default: //
				return false;
		}
	}
} // namespace NetworkReplyUtils


/*! Represents a version number.
 * A version number is a sequence of (dot separated) unsigned integers potentially followed by a suffix.
 *
 * \since 0.3.0
 */
struct VersionNumber
{
	/*! The container type holding the version segments.
	 * \sa segments
	 */
	using SegmentVector = std::vector< unsigned int >;

	/*! The numeric segments that make up the version number.
	 */
	SegmentVector segments;
	/*! The non-numeric suffix of the version number.
	 */
	QString suffix;

	/*! \return `"."` which is the string separating the version segments in the string representation of the version
	 * number.
	 */
	static QString segmentSeparator()
	{
		const auto separator = QStringLiteral( "." );
		return separator;
	}

	/*! Creates an empty VersionNumber.
	 */
	VersionNumber() {}

	/*! Creates a VersionNumber from three segments.
	 * \param major The major version number.
	 * \param minor The minor version number.
	 * \param patch The patch version number.
	 * \param suffix An	optional version suffix.
	 */
	explicit VersionNumber( unsigned int major, unsigned int minor, unsigned int patch, const QString& suffix = QString() )
	{
		segments.push_back( major );
		segments.push_back( minor );
		segments.push_back( patch );
		this->suffix = suffix;
	}

	/*! Creates a VersionNumber from a string representation.
	 * \param versionStr The string representing the version number.
	 * \return A VersionNumber object corresponding to the \p versionStr or an
	 * empty VersionNumber object if the \p versionStr could not be parsed.
	 */
	static VersionNumber fromString( const QString& versionStr )
	{
		VersionNumber version;
		const auto split = versionStr.split( segmentSeparator() );

		version.segments.reserve( static_cast< SegmentVector::size_type >( split.size() ) );

		bool converted = true;
		auto iter = split.cbegin();
		const auto splitEnd = split.cend();
		for ( ; iter != splitEnd; ++iter )
		{
			const unsigned int number = iter->toUInt( &converted );
			if ( ! converted )
				break;
			version.segments.push_back( number );
		}

		if ( ! converted )
		{
			// There is a suffix
			const auto& lastSegment = *iter;
			const QRegularExpression digitRegEx( QStringLiteral( "^\\d+" ) );
			const QRegularExpressionMatch match = digitRegEx.match( lastSegment );
			if ( match.hasMatch() )
				version.segments.push_back( match.captured().toUInt() );
			version.suffix = lastSegment.mid( match.capturedLength() );
		}

		return version;
	}

	/*! \return The string representation of this version number.
	 */
	QString toString() const
	{
		QString result;
		const auto& segs = segments;
		auto segIter = segs.begin();
		const auto segsEnd = segs.end();
		for ( ; segIter != segsEnd; ++segIter )
			result += QString::number( *segIter ) + segmentSeparator();
		result.chop( segmentSeparator().size() );
		result += suffix;
		return result;
	}

	/*! Compares two VersionNumbers for equality.
	 * \param left One VersionNumber.
	 * \param right Another VersionNumber.
	 * \return \c true if \p left and \p right represent the same version number.
	 * \note Missing parts in a VersionNumber are interpreted as 0.
	 */
	friend bool operator==( const VersionNumber& left, const VersionNumber& right )
	{
		if ( &left == &right )
			return true;

		auto leftSegments = left.segments;
		auto rightSegments = right.segments;

		const auto maxSize = std::max( leftSegments.size(), rightSegments.size() );
		leftSegments.resize( maxSize );
		rightSegments.resize( maxSize );

		return leftSegments == rightSegments && left.suffix == right.suffix;
	}

	/*! Compares two VersionNumbers for inequality.
	 * \param left One VersionNumber.
	 * \param right Another VersionNumber.
	 * \return \c true if \p left and \p right represent different version numbers.
	 * \note Missing parts in a VersionNumber are interpreted as 0.
	 */
	friend bool operator!=( const VersionNumber& left, const VersionNumber& right )
	{
		return ! ( left == right );
	}

	/*! Compares if a VersionNumber is lesser than another VersionNumber.
	 * \param left One VersionNumber.
	 * \param right Another VersionNumber.
	 * \return \c true if \p left is lesser than \p right.
	 * \note Missing parts in a VersionNumber are interpreted as 0.
	 */
	friend bool operator<( const VersionNumber& left, const VersionNumber& right )
	{
		auto leftIter = left.segments.begin();
		const auto leftEnd = left.segments.end();
		auto rightIter = right.segments.begin();
		const auto rightEnd = right.segments.end();

		while ( leftIter != leftEnd || rightIter != rightEnd )
		{
			const auto leftPart = ( leftIter != leftEnd ) ? *leftIter : 0;
			const auto rightPart = ( rightIter != rightEnd ) ? *rightIter : 0;

			if ( leftPart < rightPart )
				return true;
			if ( leftPart > rightPart )
				return false;

			if ( leftIter != leftEnd )
				++leftIter;
			if ( rightIter != rightEnd )
				++rightIter;
		}

		if ( left.suffix.isEmpty() && ! right.suffix.isEmpty() )
			return false;
		if ( ! left.suffix.isEmpty() && right.suffix.isEmpty() )
			return true;
		return left.suffix < right.suffix;
	}

	/*! Compares if a VersionNumber is greater than another VersionNumber.
	 * \param left One VersionNumber.
	 * \param right Another VersionNumber.
	 * \return \c true if \p left is greater than \p right.
	 * \note Missing parts in a VersionNumber are interpreted as 0.
	 */
	friend bool operator>( const VersionNumber& left, const VersionNumber& right )
	{
		auto leftIter = left.segments.begin();
		const auto leftEnd = left.segments.end();
		auto rightIter = right.segments.begin();
		const auto rightEnd = right.segments.end();

		while ( leftIter != leftEnd || rightIter != rightEnd )
		{
			const auto leftPart = ( leftIter != leftEnd ) ? *leftIter : 0;
			const auto rightPart = ( rightIter != rightEnd ) ? *rightIter : 0;

			if ( leftPart > rightPart )
				return true;
			if ( leftPart < rightPart )
				return false;

			if ( leftIter != leftEnd )
				++leftIter;
			if ( rightIter != rightEnd )
				++rightIter;
		}

		if ( left.suffix.isEmpty() && ! right.suffix.isEmpty() )
			return true;
		if ( ! left.suffix.isEmpty() && right.suffix.isEmpty() )
			return false;
		return left.suffix > right.suffix;
	}

	/*! Compares if a VersionNumber is greater than or equal to another VersionNumber.
	 * \param left One VersionNumber.
	 * \param right Another VersionNumber.
	 * \return \c true if \p left is greater than or equal to \p right.
	 * \note Missing parts in a VersionNumber are interpreted as 0.
	 */
	friend bool operator>=( const VersionNumber& left, const VersionNumber& right )
	{
		return ! ( left < right );
	}

	/*! Compares if a VersionNumber is lesser than or equal to another VersionNumber.
	 * \param left One VersionNumber.
	 * \param right Another VersionNumber.
	 * \return \c true if \p left is lesser than or equal to \p right.
	 * \note Missing parts in a VersionNumber are interpreted as 0.
	 */
	friend bool operator<=( const VersionNumber& left, const VersionNumber& right )
	{
		return ! ( left > right );
	}
};

/*! Wrapper class providing a common interface for string/text decoding.
 *
 * This class is an implementation of the bridge pattern combined with the adapter
 * pattern (wrapper). Its implementation is either realized by a QTextCodec or
 * by a QStringDecoder. Which implementation is used depends on the availability.
 * If both are available, QTextCodec is used unless the StringDecoder is
 * constructed with a QStringDecoder.
 *
 * This class mainly exists to provide compatibility for both %Qt 5 and %Qt 6 since
 * the QTextCodec class was deprecated in %Qt 6.
 *
 * \warning A StringDecoder must be valid to be used. Trying to decode with an invalid
 * decoder might result in undefined behavior. See isValid().
 *
 * \since 0.5.0
 */
class StringDecoder
{
public:
	/*! Creates a StringDecoder with an optional codec.
	 *
	 * \param codec The name of the codec which this StringDecoder should decode.
	 * If \p codec is empty or unknown to the implementation, the StringDecoder will
	 * be invalid.
	 *
	 * \sa isValid()
	 * \sa setCodec()
	 */
	explicit StringDecoder( const QString& codec = QString() )
	{
		if ( ! codec.isEmpty() )
			setCodec( codec );
	}

#if defined( MOCKNETWORKACCESSMANAGER_QT_HAS_TEXTCODEC )
	/*! Creates a StringDecoder which uses the given QTextCodec as implementation.
	 *
	 * \param codec The QTextCodec to be used to decode the data.
	 * If \p codec is `NULL`, the constructed StringDecoder will be invalid.
	 */
	explicit StringDecoder( QTextCodec* codec );
#endif

#if QT_VERSION >= QT_VERSION_CHECK( 6,0,0 )
	/*! Creates a StringDecoder which uses the given QStringDecoder as implementation.
	 *
	 * \note Since StringDecoder is stateless, it will call QStringDecoder::resetState()
	 * on the \p decoder every time before it decodes data.
	 *
	 * \param decoder The QStringDecoder to be used to decode the data. If \p decoder
	 * contains a `nullptr`, the constructed StringDecoder will be invalid.
	 */
	explicit StringDecoder( std::unique_ptr< QStringDecoder >&& decoder );
#endif

	/*! Creates a copy of another StringDecoder.
	 *
	 * The constructed StringDecoder will use the same implementation
	 * as \p other.
	 *
	 * \param other The StringDecoder to be copied.
	 */
	StringDecoder( const StringDecoder& other )
	{
		if ( other.m_impl )
			m_impl = other.m_impl->clone();
	}

	/*! Creates a StringDecoder by moving another one.
	 *
	 * \param other The StringDecoder to be moved.
	 */
	StringDecoder( StringDecoder&& other ) = default;

	/*! Destroys this StringDecoder and its implementation.
	 */
	~StringDecoder()
	{
		// unique_ptr takes care of clean up
		// This destructor just exists to fix SonarCloud cpp:S3624
	}

	/*! Makes this StringDecoder use the same implementation as another one.
	 *
	 * \param other The StringDecoder whose implementation is copied.
	 * \return A reference to this StringDecoder.
	 */
	StringDecoder& operator=( StringDecoder other )
	{
		m_impl.swap( other.m_impl );
		return *this;
	}

	/*! Makes this StringDecoder use the implementation of another one.
	 *
	 * \param other The StringDecoder whose implementation is moved.
	 * \return A reference to this StringDecoder.
	 */
	StringDecoder& operator=( StringDecoder&& other ) = default;

	/*! Checks if this StringDecoder can decode data.
	 *
	 * Trying to decode data with an invalid StringDecoder may result in undefined
	 * behavior.
	 *
	 * \return \c true if this StringDecoder contains a valid implementation
	 * and can decode data.
	 */
	bool isValid() const
	{
		return m_impl && m_impl->isValid();
	}

	/*! Sets the codec used by this StringDecoder.
	 *
	 * \param codec The name of the codec to be used to decode data.
	 * If \p codec is empty or unknown to the implementation, this StringDecoder
	 * becomes invalid.
	 *
	 * \sa isValid()
	 */
	void setCodec( const QString& codec )
	{
		ensureImpl();
		m_impl->setCodec( codec );
	}

	/*! Sets the codec by trying to detect the codec of given data.
	 *
	 * If the codec cannot be detected and \p fallbackCodec is empty or
	 * unknown to the implementation, this StringDecoder becomes invalid.
	 *
	 * \param data The data whose codec should be detected.
	 * \param fallbackCodec If the codec of \p data cannot be detected,
	 * this \p fallbackCodec is used instead.
	 *
	 * \sa isValid()
	 */
	void setCodecFromData( const QByteArray& data, const QString& fallbackCodec )
	{
		ensureImpl();
		m_impl->setCodecFromData( data, fallbackCodec );
	}

	/*! Decodes data with the configured codec.
	 *
	 * \warning The StringDecoder must be valid when calling decode() or undefined
	 * behavior might be invoked.
	 *
	 * \param data The data to be decoded.
	 * \return
	 *
	 * \sa QTextCodec::toUnicode()
	 * \sa QStringDecoder::decode()
	 */
	QString decode( const QByteArray& data ) const
	{
		Q_ASSERT_X( isValid(), Q_FUNC_INFO, "Trying to use invalid StringDecoder" );
		return m_impl->decode( data );
	}

private:
	//! \cond PRIVATE_IMPLEMENTATION
	class Impl
	{
	public:
		virtual ~Impl() {}
		virtual bool isValid() const = 0;
		virtual void setCodec( const QString& codec ) = 0;
		virtual void setCodecFromData( const QByteArray& data, const QString& fallbackCodec ) = 0;
		virtual QString decode( const QByteArray& data ) const = 0;
		virtual std::unique_ptr< Impl > clone() const = 0;
	};

#if defined( MOCKNETWORKACCESSMANAGER_QT_HAS_TEXTCODEC )
	class TextCodecImpl : public Impl
	{
	public:
		explicit TextCodecImpl( const QTextCodec* codec = Q_NULLPTR )
		    : m_codec( codec )
		{
		}
		bool isValid() const override
		{
			return m_codec != Q_NULLPTR;
		}
		void setCodec( const QString& codec ) override
		{
			m_codec = QTextCodec::codecForName( codec.toUtf8() );
		}
		void setCodecFromData( const QByteArray& data, const QString& fallbackCodec ) override
		{
			m_codec = QTextCodec::codecForUtfText( data, Q_NULLPTR );
			if ( ! m_codec )
				setCodec( fallbackCodec );
		}
		QString decode( const QByteArray& data ) const override
		{
			Q_ASSERT( m_codec );
			return m_codec->toUnicode( data );
		}
		std::unique_ptr< Impl > clone() const override
		{
			return std::unique_ptr< Impl >( new TextCodecImpl{ m_codec } );
		}

	private:
		const QTextCodec* m_codec;
	};
#endif

#if QT_VERSION >= QT_VERSION_CHECK( 6,0,0 )
	class StringDecoderImpl : public Impl
	{
	public:
		StringDecoderImpl() = default;
		explicit StringDecoderImpl( std::unique_ptr< QStringDecoder >&& decoder )
		    : m_decoder( std::move( decoder ) )
		{
		}
		bool isValid() const override
		{
			return m_decoder && m_decoder->isValid();
		}
		void setCodec( const QString& codec ) override
		{
			auto encoding = QStringConverter::encodingForName( codec.toUtf8().constData() );
			if ( encoding )
			{
				constructQStringDecoder( encoding.value() );
				return;
			}
			m_decoder.reset();
		}
		void setCodecFromData( const QByteArray& data, const QString& fallbackCodec ) override
		{
			auto encoding = QStringConverter::encodingForData( data );
			if ( encoding )
			{
				constructQStringDecoder( encoding.value() );
				return;
			}
			setCodec( fallbackCodec );
		}
		QString decode( const QByteArray& data ) const override
		{
			Q_ASSERT( m_decoder );
			m_decoder->resetState();
			return m_decoder->decode( data );
		}
		std::unique_ptr< Impl > clone() const override
		{
			if ( ! isValid() )
				return std::make_unique< StringDecoderImpl >();

			const auto* encodingName = m_decoder->name();
			Q_ASSERT( encodingName );
			const auto encoding = QStringConverter::encodingForName( encodingName );
			Q_ASSERT( encoding );
			auto cloned = std::make_unique< StringDecoderImpl >();
			cloned->constructQStringDecoder( encoding.value() );
			return cloned;
		}

	private:
		void constructQStringDecoder( QStringConverter::Encoding encoding )
		{
			m_decoder = std::make_unique< QStringDecoder >( encoding, QStringConverter::Flag::Stateless );
		}
		std::unique_ptr< QStringDecoder > m_decoder;
	};
#endif
	//! \endcond

private:
	void ensureImpl()
	{
		if ( ! m_impl )
		{
#if defined( MOCKNETWORKACCESSMANAGER_QT_HAS_TEXTCODEC )
			m_impl.reset( new TextCodecImpl() );
#else
			m_impl.reset( new StringDecoderImpl() );
#endif
		}
	}

	std::unique_ptr< Impl > m_impl;
};

class Rule;
class MockReplyBuilder;
class NetworkReply;
template< class Base >
class Manager;
/*! \internal Implementation details
 */
namespace detail {
	class ManagerInterface;
} // namespace detail


/*! QList of QByteArray. */
using ByteArrayList = QList< QByteArray >;
/*! QSet of [QNetworkRequest::Attribute].
 * [QNetworkRequest::Attribute]: http://doc.qt.io/qt-5/qnetworkrequest.html#Attribute-enum
 */
using AttributeSet = QSet< QNetworkRequest::Attribute >;
/*! QHash holding [QNetworkRequest::Attributes] and their corresponding values.
 * \sa QNetworkRequest::attribute()
 * [QNetworkRequest::Attributes]: http://doc.qt.io/qt-5/qnetworkrequest.html#Attribute-enum
 * \since 0.11.0
 */
using AttributeHash = QHash< QNetworkRequest::Attribute, QVariant >;
/*! QHash holding [QNetworkRequest::KnowHeaders] and their corresponding values.
 * \sa QNetworkRequest::header()
 * [QNetworkRequest::KnowHeaders]: http://doc.qt.io/qt-5/qnetworkrequest.html#KnownHeaders-enum
 */
using HeaderHash = QHash< QNetworkRequest::KnownHeaders, QVariant >;
/*! QSet holding [QNetworkRequest::KnowHeaders].
 * [QNetworkRequest::KnowHeaders]: http://doc.qt.io/qt-5/qnetworkrequest.html#KnownHeaders-enum
 */
using KnownHeadersSet = QSet< QNetworkRequest::KnownHeaders >;
/*! QHash holding raw headers and their corresponding values.
 * \sa QNetworkRequest::rawHeader()
 */
using RawHeaderHash = QHash< QByteArray, QByteArray >;
/*! QHash holding query parameter names and their corresponding values.
 * \sa QUrlQuery
 */
using QueryParameterHash = QHash< QString, QString >;
/*! QHash holding query parameter names and their corresponding values.
 * \sa QUrlQuery
 * \since 0.4.0
 */
using MultiValueQueryParameterHash = QHash< QString, QStringList >;
/*! QVector of QRegularExpression QPairs.
 */
using RegExPairVector = QVector< QPair< QRegularExpression, QRegularExpression > >;
/*! QList of QNetworkReply::RawHeaderPairs.
 * \since 0.11.0
 */
using RawHeaderPairList = QList< QNetworkReply::RawHeaderPair >;


/*! Determines the MIME type of data.
 * \param url The URL of the \p data.
 * \param data The data itself.
 * \return The MIME type of the \p data located at \p url.
 * \sa QMimeDatabase::mimeTypeForFileNameAndData()
 */
inline QMimeType guessMimeType( const QUrl& url, const QByteArray& data )
{
	const QFileInfo fileInfo( url.path() );
	return QMimeDatabase().mimeTypeForFileNameAndData( fileInfo.fileName(), data );
}

/*! \return The %Qt version used at runtime as a VersionNumber object.
 * \sa qVersion()
 * \since 0.11.0
 */
inline VersionNumber qtVersionInUse()
{
#if QT_VERSION < QT_VERSION_CHECK( 5,6,0 )
	#error MockNetworkAccessManager requires Qt 5.6.0 or later
#endif // Qt < 5.6.0

	const char* qtVersion = ::qVersion();
	return VersionNumber::fromString( QString::fromLatin1( qtVersion ) );
}

/*! Determines the behavior of the %Qt version in use.
 * This is also the default behavior of Manager objects if not overridden using Manager::setBehaviorFlags().
 * \return The BehaviorFlags matching the behavior of the %Qt version used at runtime.
 * \sa qtVersionInUse()
 * \sa [qVersion()](https://doc.qt.io/qt-5/qtglobal.html#qVersion)
 * \sa BehaviorFlag
 * \since 0.3.0
 */
inline BehaviorFlags getDefaultBehaviorFlags()
{
	const auto versionInUse = qtVersionInUse();

	if ( versionInUse >= VersionNumber( 5, 9, 3 ) )
		return Behavior_Qt_5_9_3;
	else if ( versionInUse >= VersionNumber( 5, 6, 0 ) )
		return Behavior_Qt_5_6_0;
	else
		return Behavior_Qt_5_2_0;
}

/*! Defines the possible behaviors of the Manager when a request does not match any Rule.
 *
 * By default, the Manager returns a predefined reply for unmatched requests. The reply has set
 * QNetworkReply::ContentNotFoundError and an error message indicating that the request did not
 * match any Rule.
 * The default reply can be modified via Manager::unmatchedRequestBuilder().
 */
enum UnmatchedRequestBehavior
{
	Forward,        /*!< Unmatched requests are forwarded to the next network access manager.
	                 * \sa Manager::setForwardingTargetNam()
	                 * \sa \ref page_forwarding
	                 */
	PredefinedReply /*!< The manager will return a predefined reply for unmatched requests.
	                 * \since 0.8.0 This is the default behavior.
	                 * \sa Manager::setUnmatchedRequestBuilder()
	                 */
};

/*! \internal Implementation details
 */
namespace detail {

	/*! \internal
	 * Converts a RawHeaderHash to a RawHeaderPairList.
	 * @param hash The RawHeaderHash.
	 * @return A RawHeaderPairList containing the same entries as the \p hash.
	 * The order of the entries is undefined.
	 */
	inline RawHeaderPairList rawHeaderHashToPairList( const RawHeaderHash& hash )
	{
		RawHeaderPairList result;
		result.reserve( hash.size() );
		const auto end = hash.cend();
		for ( auto iter = hash.cbegin(); iter != end; ++iter )
			result.append( QNetworkReply::RawHeaderPair( iter.key(), iter.value() ) );
		return result;
	}

	/*! Returns the default port for a given URL scheme.
	 * @param scheme A URL scheme.
	 * @return The default port for the given URL \p scheme or \c -1 if the default port
	 * for the given \p scheme is not known.
	 */
	inline int defaultPortForScheme( const QString& scheme )
	{
		const QString urlProtocol = scheme.toLower();
		if ( urlProtocol == HttpUtils::httpScheme() )
			return HttpUtils::HttpDefaultPort;
		else if ( urlProtocol == HttpUtils::httpsScheme() )
			return HttpUtils::HttpsDefaultPort;
		else if ( urlProtocol == FtpUtils::ftpScheme() )
			return FtpUtils::FtpDefaultPort;
		return -1;
	}

} // namespace detail


/*! Provides access to the request data.
 *
 * This mainly groups all the request data into a single struct for convenience.
 */
struct Request
{
	/*! The HTTP request verb.
	 */
	QNetworkAccessManager::Operation operation;
	/*! The QNetworkRequest object.
	 * This provides access to the details of the request like URL, headers and attributes.
	 */
	QNetworkRequest qRequest;
	/*! The body data.
	 */
	QByteArray body;
	/*! The timestamp when the Manager began handling the request.
	 * For requests received through the public API of QNetworkAccessManager,
	 * this can be considered the time when the Manager received the request.
	 */
	QDateTime timestamp;

	/*! Creates an invalid Request object.
	 * \sa isValid()
	 */
	Request()
	    : operation( QNetworkAccessManager::CustomOperation )
	{
	}

	/*! Creates a Request struct.
	 * \param op The Request::operation.
	 * \param req The Request:.qRequest.
	 * \param data The Request::body.
	 * \note The Request::timestamp will be set to the current date and time.
	 */
	Request( QNetworkAccessManager::Operation op, const QNetworkRequest& req, const QByteArray& data = QByteArray() )
	    : operation( op )
	    , qRequest( req )
	    , body( data )
	    , timestamp( QDateTime::currentDateTime() )
	{
	}

	/*! Creates a Request struct.
	 * \param req The Request:.qRequest.
	 * \param op The Request::operation.
	 * \param data The Request::body.
	 * \note The Request::timestamp will be set to the current date and time.
	 */
	Request( const QNetworkRequest& req,
	         QNetworkAccessManager::Operation op = QNetworkAccessManager::GetOperation,
	         const QByteArray& data = QByteArray() )
	    : operation( op )
	    , qRequest( req )
	    , body( data )
	    , timestamp( QDateTime::currentDateTime() )
	{
	}

	/*! \return \c true if the Request specifies a valid HTTP verb and the qRequest contains a valid URL.
	 * The HTTP is not valid if operation is QNetworkAccessManager::CustomOperation
	 * and the [QNetworkRequest::CustomVerbAttribute] of qRequest is empty.
	 * [QNetworkRequest::CustomVerbAttribute]: http://doc.qt.io/qt-5/qnetworkrequest.html#Attribute-enum
	 */
	bool isValid() const
	{
		return qRequest.url().isValid()
		       && ( operation != QNetworkAccessManager::CustomOperation
		            || ! qRequest.attribute( QNetworkRequest::CustomVerbAttribute ).toByteArray().trimmed().isEmpty() );
	}

	/*! Checks if two Request structs are equal.
	 * \param left One Request struct to be compared.
	 * \param right The other Request struct to be compared with \p left.
	 * \return \c true if all fields of \p left and \c right are equal (including the Request::timestamp).
	 * \c false otherwise.
	 */
	friend bool operator==( const Request& left, const Request& right )
	{
		return left.operation == right.operation  //
		       && left.qRequest == right.qRequest //
		       && left.body == right.body         //
		       && left.timestamp == right.timestamp;
	}

	/*! Checks if two Request structs differ.
	 * \param left One Request struct to be compared.
	 * \param right The other Request struct to be compared with \p left.
	 * \return \c true if at least one field of \p left and \c right differs (including the Request::timestamp).
	 * \c false if \p left and \p right are equal.
	 */
	friend bool operator!=( const Request& left, const Request& right )
	{
		return ! ( left == right );
	}

	/*! Returns the operation (HTTP verb) of the request as a string.
	 * \return The Request::operation of the Request as a QString or a null `QString()` if the operation is unknown
	 * or it is `QNetworkAccessManager::CustomOperation` but the `QNetworkRequest::CustomVerbAttribute` was not set
	 * on the Request::qRequest. For the standard operations, the verb is returned in all uppercase letters. For a
	 * `CustomOperation`, the verb is return as set in the `QNetworkRequest::CustomVerbAttribute`.
	 */
	QString verb() const
	{
		switch ( this->operation )
		{
			case QNetworkAccessManager::GetOperation:
				return QStringLiteral( "GET" );
			case QNetworkAccessManager::HeadOperation:
				return QStringLiteral( "HEAD" );
			case QNetworkAccessManager::PostOperation:
				return QStringLiteral( "POST" );
			case QNetworkAccessManager::PutOperation:
				return QStringLiteral( "PUT" );
			case QNetworkAccessManager::DeleteOperation:
				return QStringLiteral( "DELETE" );
			case QNetworkAccessManager::CustomOperation:
				return this->qRequest.attribute( QNetworkRequest::CustomVerbAttribute ).toString();
			// LCOV_EXCL_START
			default:
				qCWarning( log ) << "Unknown operation:" << this->operation;
				return QString();
				// LCOV_EXCL_STOP
		}
	}
};

/*! QList of Request structs.*/
using RequestList = QList< Request >;

/*! Holds the information necessary to make a signal connection.
 *
 * \sa QObject::connect()
 */
class SignalConnectionInfo
{
public:
	/*! Creates an invalid SignalConnectionInfo object.
	 */
	SignalConnectionInfo()
	    : m_sender( Q_NULLPTR )
	    , m_connectionType( Qt::AutoConnection )
	{
	}

	/*! Creates a SignalConnectionInfo for a given object and signal.
	 *
	 * \param sender The QObject which is the sender of the signal.
	 * \param metaSignal The QMetaMethod of the signal.
	 * \param connectionType The type of the connection.
	 */
	SignalConnectionInfo( QObject* sender,
	                      const QMetaMethod& metaSignal,
	                      Qt::ConnectionType connectionType = Qt::AutoConnection )
	    : m_sender( sender )
	    , m_signal( metaSignal )
	    , m_connectionType( connectionType )
	{
	}

	/*! \return The sender QObject.
	 */
	QObject* sender() const
	{
		return m_sender;
	}

	/*! \return The QMetaMethod of the signal.
	 */
	// @sonarcloud-exclude-start
	QMetaMethod signal() const
	// @sonarcloud-exclude-end
	{
		return m_signal;
	}

	/*! \return The type of the connection.
	 */
	Qt::ConnectionType connectionType() const
	{
		return m_connectionType;
	}

	/*! \return \c true if this SignalConnectionInfo object contains information allowing to make a valid signal
	 * connection. This means that there must be a sender object set and a signal which belongs to this sender object.
	 */
	bool isValid() const
	{
		return m_sender                                        //
		       && m_signal.isValid()                           //
		       && m_signal.methodType() == QMetaMethod::Signal //
		       && m_sender->metaObject()->method( m_signal.methodIndex() ) == m_signal;
	}

	/*! Creates a connection to the signal described by this %SignalConnectionInfo.
	 *
	 * \note If this %SignalConnectionInfo object is not valid, the connection will not be established and an invalid
	 * QMetaObject::Connection object is returned.
	 *
	 * \param receiver The receiver QObject.
	 * \param slotOrSignal The QMetaMethod of the signal or slot which is connected to the signal described by the this
	 * %SignalConnectionInfo.
	 * \return The QMetaObject::Connection object as returned by QObject::connect().
	 *
	 * \sa isValid()
	 * \sa QObject::connect()
	 */
	QMetaObject::Connection connect( QObject* receiver, const QMetaMethod& slotOrSignal ) const
	{
		return QObject::connect( m_sender, m_signal, receiver, slotOrSignal, m_connectionType );
	}

	/*! Compares two SignalConnectionInfo objects for equality.
	 *
	 * \param left One SignalConnectionInfo object.
	 * \param right Another SignalConnectionInfo object.
	 * \return \c true if \p left and \p right contain the same data.
	 */
	friend bool operator==( const SignalConnectionInfo& left, const SignalConnectionInfo& right )
	{
		return left.m_sender == right.m_sender    //
		       && left.m_signal == right.m_signal //
		       && left.m_connectionType == right.m_connectionType;
	}

	/*! Compares two SignalConnectionInfo objects for inequality.
	 *
	 * \param left One SignalConnectionInfo object.
	 * \param right Another SignalConnectionInfo object.
	 * \return \c true if \p left and \p right contain different data.
	 */
	friend bool operator!=( const SignalConnectionInfo& left, const SignalConnectionInfo& right )
	{
		return ! ( left == right );
	}

private:
	QObject* m_sender;
	QMetaMethod m_signal;
	Qt::ConnectionType m_connectionType;
};

/*! \internal Implementation details
 */
namespace detail {

	inline bool usesSafeRedirectCustomRequestMethod( const Request& request );

	/* RFC-7231 defines the request methods GET, HEAD, OPTIONS, and TRACE to be safe
	 * for automatic redirection using the same method.
	 * See https://tools.ietf.org/html/rfc7231#section-6.4
	 * and https://tools.ietf.org/html/rfc7231#section-4.2.1
	 */
	inline bool usesSafeRedirectRequestMethod( const Request& request )
	{
		switch ( request.operation )
		{
			case QNetworkAccessManager::GetOperation:
			case QNetworkAccessManager::HeadOperation:
				return true;
			case QNetworkAccessManager::CustomOperation:
				return usesSafeRedirectCustomRequestMethod( request );
			default:
				return false;
		}
	}

	inline bool usesSafeRedirectCustomRequestMethod( const Request& request )
	{
		const QString customVerb = request.qRequest.attribute( QNetworkRequest::CustomVerbAttribute ).toString().toLower();
		return ( customVerb == QLatin1String( "options" ) || customVerb == QLatin1String( "trace" ) );
	}

	inline bool isDataUrlRequest( const Request& request )
	{
		return request.qRequest.url().scheme() == DataUrlUtils::dataScheme();
	}

	inline bool automaticRedirectFollowingEnabled( const QNetworkAccessManager& manager, const Request& request )
	{
#if QT_VERSION >= QT_VERSION_CHECK( 5,9,0 )
		const QVariant redirectPolicy = request.qRequest.attribute( QNetworkRequest::RedirectPolicyAttribute );
		if ( redirectPolicy.isValid() )
			return redirectPolicy.toInt() != static_cast< int >( QNetworkRequest::ManualRedirectPolicy );
#endif // Qt >= 5.9.0
#if QT_VERSION < QT_VERSION_CHECK( 5,15,2 )
		const QVariant followRedirectAttribute = request.qRequest.attribute( QNetworkRequest::FollowRedirectsAttribute );
		if ( followRedirectAttribute.isValid() )
			return followRedirectAttribute.toBool();
#endif // Qt < 5.15.2
#if QT_VERSION >= QT_VERSION_CHECK( 5,9,0 )
		const QNetworkRequest::RedirectPolicy namRedirectPolicy = manager.redirectPolicy();
		return namRedirectPolicy != QNetworkRequest::ManualRedirectPolicy;
#endif // Qt >= 5.9.0
		Q_UNUSED( manager )
		Q_UNUSED( request )
		return false;
	}

	/*! \internal
	 * Updates the state of a QNetworkAccessManager according to reply headers.
	 * This includes updating cookies and HSTS entries.
	 */
	class ReplyHeaderHandler : public QObject
	{
		Q_OBJECT

	public:
		ReplyHeaderHandler( QNetworkAccessManager* manager, QObject* parent = nullptr )
		    : QObject( parent )
		    , m_manager( manager )
		{
		}

		~ReplyHeaderHandler() override = default;

	public Q_SLOTS:
		void handleReplyHeaders( QNetworkReply* reply )
		{
			Q_ASSERT( reply );

			handleKnownHeaders( reply );
			handleRawHeaders( reply );
		}

	private:
		void handleKnownHeaders( QNetworkReply* reply )
		{
			handleSetCookieHeader( reply );
		}

		void handleSetCookieHeader( QNetworkReply* reply )
		{
			QNetworkRequest request = reply->request();
			const bool saveCookies = requestSavesCookies( request );

			QNetworkCookieJar* cookieJar = m_manager->cookieJar();
			if ( saveCookies && cookieJar )
			{
				const QList< QNetworkCookie > cookies = reply->header( QNetworkRequest::SetCookieHeader )
				                                            .value< QList< QNetworkCookie > >();
				if ( ! cookies.isEmpty() )
					cookieJar->setCookiesFromUrl( cookies, reply->url() );
			}
		}

		static bool requestSavesCookies( const QNetworkRequest& request )
		{
			const auto defaultValue = static_cast< int >( QNetworkRequest::Automatic );
			const auto saveCookiesInt = request.attribute( QNetworkRequest::CookieSaveControlAttribute, defaultValue )
			                                .toInt();
			return static_cast< QNetworkRequest::LoadControl >( saveCookiesInt ) == QNetworkRequest::Automatic;
		}

		void handleRawHeaders( QNetworkReply* reply )
		{
			const auto& rawHeaderPairs = reply->rawHeaderPairs();
			auto headerIter = rawHeaderPairs.cbegin();
			const auto headerEnd = rawHeaderPairs.cend();
			for ( ; headerIter != headerEnd; ++headerIter )
			{
				// header field-name is ASCII according to RFC 7230 3.2
				const auto headerName = headerIter->first.toLower();

#if ( QT_VERSION >= QT_VERSION_CHECK( 5, 9, 0 ) )
				const QByteArray stsHeader( "strict-transport-security" );
				if ( headerName == stsHeader )
				{
					handleStsHeader( headerIter->second, reply );
				}
#endif // Qt >= 5.9.0
			}
		}

#if ( QT_VERSION >= QT_VERSION_CHECK( 5, 9, 0 ) )
		void handleStsHeader( const QByteArray& headerValue, const QNetworkReply* reply )
		{
			const auto stsPolicies = HttpUtils::splitCommaSeparatedList( QString::fromLatin1( headerValue ) );
			auto stsPolicyIter = stsPolicies.constBegin();
			const auto stsPolicyEnd = stsPolicies.constEnd();
			for ( ; stsPolicyIter != stsPolicyEnd; ++stsPolicyIter )
			{
				/* If the header has an invalid syntax, we ignore it and continue
				 * until we find a valid STS policy.
				 */
				if ( processStsPolicy( stsPolicyIter->toLatin1(), reply->url() ) )
					break; // following STS policies are ignored
				continue;
			}
		}

		bool processStsPolicy( const QByteArray& header, const QUrl& host )
		{
			const auto headerData = QString::fromLatin1( header );
			const auto directives = headerData.split( QChar::fromLatin1( ';' ) );

			QHstsPolicy policy;
			policy.setHost( host.host() );

			QSet< QString > foundDirectives;

			auto directiveIter = directives.cbegin();
			const auto directiveEnd = directives.cend();
			for ( ; directiveIter != directiveEnd; ++directiveIter )
			{
				const auto cleanDirective = HttpUtils::whiteSpaceCleaned( *directiveIter );
				const auto directiveSplit = splitStsDirective( cleanDirective );
				const auto& directiveName = directiveSplit.first;
				const auto& directiveValue = directiveSplit.second;

				if ( foundDirectives.contains( directiveName ) )
					return false; // Invalid header: duplicate directive
				foundDirectives.insert( directiveName );

				if ( ! processStsDirective( policy, directiveName, directiveValue ) )
					return false;
			}

			if ( ! foundDirectives.contains( maxAgeDirectiveName() ) )
				return false; // Invalid header: missing required max-age directive

			m_manager->addStrictTransportSecurityHosts( QVector< QHstsPolicy >() << policy );
			return true;
		}

		static QLatin1String maxAgeDirectiveName()
		{
			return QLatin1String( "max-age" );
		}

		static QPair< QString, QString > splitStsDirective( const QString& directive )
		{
			const QRegularExpression basicDirectiveRegEx( QStringLiteral( "^([^=]*)=?(.*)$" ) );

			QRegularExpressionMatch match;

			match = basicDirectiveRegEx.match( directive );
			// This should be impossible since basicDirectiveRegEx matches everything
			Q_ASSERT_X( match.hasMatch(), Q_FUNC_INFO, "Could not parse directive." );

			const QString directiveName = HttpUtils::whiteSpaceCleaned( match.captured( 1 ) ).toLower();
			const QString rawDirectiveValue = HttpUtils::whiteSpaceCleaned( match.captured( 2 ) );
			const QString directiveValue = HttpUtils::isValidToken( rawDirectiveValue )
			                                   ? rawDirectiveValue
			                                   : HttpUtils::unquoteString( rawDirectiveValue );

			return ::qMakePair( directiveName, directiveValue );
		}

		static bool processStsDirective( QHstsPolicy& policy, const QString& directiveName, const QString& directiveValue )
		{
			if ( directiveName == maxAgeDirectiveName() )
			{
				return processStsMaxAgeDirective( policy, directiveValue );
			}

			if ( directiveName == QLatin1String( "includesubdomains" ) )
			{
				policy.setIncludesSubDomains( true );
				return true;
			}

			// else we check if the directive is legal at all
			if ( ! HttpUtils::isValidToken( directiveName ) )
				return false; // Invalid header: illegal directive name

			if ( ! HttpUtils::isValidToken( directiveValue ) && ! HttpUtils::isValidQuotedString( directiveValue ) )
				return false; // Invalid header: illegal directive value

			// Directive seems legal but simply unknown. So we ignore it.
			return true;
		}

		static bool processStsMaxAgeDirective( QHstsPolicy& policy, const QString& directiveValue )
		{
			const QRegularExpression maxAgeValueRegEx( QStringLiteral( "\\d+" ) );

			const QRegularExpressionMatch match = maxAgeValueRegEx.match( directiveValue );
			if ( ! match.hasMatch() )
				return false; // Invalid header: incorrect max-age value
			const qint64 maxAge = match.captured( 0 ).toLongLong();
			policy.setExpiry( QDateTime::currentDateTimeUtc().addSecs( maxAge ) );
			return true;
		}
#endif // Qt >= 5.9.0

		QPointer< QNetworkAccessManager > m_manager;
	};

	/*! \internal
	 * Simulates a connection to a dedicated target (host+port).
	 */
	class Connection : public QObject
	{
		Q_OBJECT

		friend class ConnectionPool;

	public:
		using Ptr = std::unique_ptr< Connection >;

		Connection( const QString& protocol, const QString& target, QObject* parent = nullptr )
		    : QObject( parent )
		    , m_protocol{ protocol }
		    , m_target{ target }
		{
		}

		QString protocol() const
		{
			return m_protocol;
		}

		QString target() const
		{
			return m_target;
		}

		bool isConnected() const
		{
			return m_isConnected;
		}

		bool isNew() const
		{
			return m_isNew;
		}

		void release()
		{
			m_lastUsed = QDateTime::currentDateTime();
			m_isInUse = false;
			m_isNew = false;
			Q_EMIT released( this );
		}

		void setKeepAlive( bool keepAlive )
		{
			m_keepAlive = keepAlive;
		}

		bool keepAlive() const
		{
			return m_keepAlive;
		}

	Q_SIGNALS:
		void connected();
		void released( Connection* connection );

	protected:
		void setInUse()
		{
			m_lastUsed = QDateTime::currentDateTime();
			m_isInUse = true;
		}

		bool isInUse() const
		{
			return m_isInUse;
		}

		void setConnected()
		{
			m_isConnected = true;
			m_isNew = true;
			QMetaObject::invokeMethod( this, "connected", Qt::QueuedConnection );
		}

	private:
		QDateTime m_lastUsed;
		bool m_isInUse = true;
		bool m_isConnected = false;
		bool m_isNew = true;
		bool m_keepAlive = true;
		QString m_protocol;
		QString m_target;
	};

	/*! \internal
	 * Manages connections to ensure proper behavior of connection reusing (emission of QNetworkReply::encrypted() signal).
	 */
	class ConnectionPool : public QObject
	{
		Q_OBJECT

	public:
		ConnectionPool( unsigned int maxParallelConnectionsPerTarget = 6,
		                unsigned int connectionTimeoutInMilliSeconds = 0,
		                QObject* parent = nullptr )
		    : QObject( parent )
		    , m_maxParallelConnectionsPerTarget{ maxParallelConnectionsPerTarget }
		    , m_connectionTimeoutInMilliSeconds{ connectionTimeoutInMilliSeconds }
		{
		}

		unsigned int maxParallelConnectionsPerTarget() const
		{
			return m_maxParallelConnectionsPerTarget;
		}

		void setMaxParallelConnectionsPerTarget( unsigned int maxParallelConnectionsPerTarget )
		{
			m_maxParallelConnectionsPerTarget = maxParallelConnectionsPerTarget;
		}

		unsigned int connectionTimeout() const
		{
			return m_connectionTimeoutInMilliSeconds;
		}

		void setConnectionTimeout( unsigned int connectionTimeoutInMilliSeconds )
		{
			m_connectionTimeoutInMilliSeconds = connectionTimeoutInMilliSeconds;
		}

		Connection* getConnection( const QUrl& requestUrl )
		{
			if ( HttpUtils::isHttpScheme( requestUrl ) )
				return createHttpConnection( requestUrl );

			return createNonHttpConnection( requestUrl );
		}

		void releaseUnusedConnections()
		{
			auto iter = m_connections.begin();
			const auto end = m_connections.end();

			while ( iter != end )
			{
				removeAbandonedConnections( iter->second );
				if ( iter->second.empty() )
					iter = m_connections.erase( iter );
				else
					++iter;
			}
		}

	private:
		using ConnectionList = std::vector< Connection::Ptr >;
		using ConnectionQueue = std::deque< Connection::Ptr >;
		struct ConnectionSearchResult
		{
			bool found;
			ConnectionList::iterator iter;
		};

		Connection* createHttpConnection( const QUrl& requestUrl )
		{
			const auto protocol = requestUrl.scheme();
			const auto target = connectionTarget( requestUrl );

			auto* connection = findUsableConnection( protocol, target );

			if ( connection )
				return connection;

			return enqueueNewConnection( protocol, target );
		}

		static QString connectionTarget( const QUrl& url )
		{
			const auto port = url.port() == -1 ? detail::defaultPortForScheme( url.scheme() ) : url.port();
			return url.host() + QStringLiteral( ":" ) + QString::number( port );
		}

		Connection* findUsableConnection( const QString& protocol, const QString& target )
		{
			if ( m_connections.find( target ) == m_connections.end() )
			{
				auto connections = ConnectionList{};
				connections.reserve( m_maxParallelConnectionsPerTarget );
				m_connections[ target ] = std::move( connections );
			}
			auto& connections = m_connections[ target ];

			sortConnectionList( connections );

			for ( auto&& connection : detail::asConst( connections ) )
			{
				if ( ! connection->isInUse() )
				{
					if ( ! connection->keepAlive()
					     || ( m_connectionTimeoutInMilliSeconds > 0
					          && connection->m_lastUsed.msecsTo( QDateTime::currentDateTime() )
					                 > m_connectionTimeoutInMilliSeconds ) )
						connection->setConnected(); // "Reconnect"
					connection->setInUse();
					return connection.get();
				}
			}

			if ( connections.size() < m_maxParallelConnectionsPerTarget )
			{
				connections.push_back( createNewConnection( protocol, target ) );
				const auto& connection = connections.back();
				connection->setConnected();
				return connection.get();
			}

			return nullptr;
		}

		static void sortConnectionList( ConnectionList& connections )
		{
			std::sort( connections.begin(),
			           connections.end(),
			           []( const Connection::Ptr& left, const Connection::Ptr& right ) {
				           return left->m_lastUsed < right->m_lastUsed;
			           } );
		}

		Connection::Ptr createNewConnection( const QString& protocol, const QString& target ) const
		{
			Connection::Ptr connection{ new Connection{ protocol, target } };
			connection->setInUse();
			QObject::connect( connection.get(), &Connection::released, this, &ConnectionPool::releaseConnection );
			return connection;
		}

		Connection* enqueueNewConnection( const QString& protocol, const QString& target )
		{
			auto& queue = m_pendingConnections[ target ];
			queue.push_back( createNewConnection( protocol, target ) );
			return queue.back().get();
		}

		Connection* createNonHttpConnection( const QUrl& targetUrl )
		{
			m_nonHttpConnections.push_back( createNewConnection( targetUrl.scheme(), targetUrl.toDisplayString() ) );
			const auto& connection = m_nonHttpConnections.back();
			connection->setConnected();
			return connection.get();
		}

		static void removeAbandonedConnections( ConnectionQueue& queue )
		{
			while ( ! queue.empty() && ! queue.front()->isInUse() )
				queue.pop_front();
		}

		static void removeAbandonedConnections( ConnectionList& list )
		{
			const auto eraseStart = std::remove_if( list.begin(), list.end(), []( const Connection::Ptr& connection ) {
				return ! connection->isInUse();
			} );
			list.erase( eraseStart, list.end() );
		}

		ConnectionSearchResult findActiveConnection( const Connection* connection )
		{
			auto& connections = m_connections.at( connection->target() );

			auto iter = std::find_if(
			    connections.begin(),
			    connections.end(),
			    [ connection ]( const Connection::Ptr& candidate ) { return candidate.get() == connection; } );

			return ConnectionSearchResult{ iter != connections.end(), iter };
		}

	private Q_SLOTS:
		void releaseConnection( Connection* connection )
		{
			if ( HttpUtils::isHttpScheme( connection->protocol() ) )
				releaseHttpConnection( connection );
			else
			{
				/* We will potentially delete the `connection` so we need to do this "later"
				 * and not while in a slot connected to the `connection`, else it will crash.
				 */
				QMetaObject::invokeMethod( this, "releaseNonHttpConnection", Qt::QueuedConnection );
			}
		}

	private:
		void releaseHttpConnection( Connection* connection )
		{
			if ( connection->isConnected()
			     && m_pendingConnections.find( connection->target() ) != m_pendingConnections.end() )
			{
				auto& pendingConnections = m_pendingConnections[ connection->target() ];
				removeAbandonedConnections( pendingConnections );
				if ( ! pendingConnections.empty() )
				{
					auto connectionSearchResult = findActiveConnection( connection );
					Q_ASSERT_X( connectionSearchResult.found, Q_FUNC_INFO, "Could not find provided connection" );
					if ( connectionSearchResult.found )
					{
						// Prepare the pending connection
						auto nextConnection = std::move( pendingConnections.front() );
						pendingConnections.pop_front();
						nextConnection->setInUse();
						nextConnection->setConnected();

						// Put the pending connection in place of the released one and delete the released one
						auto& releasedConnection = *connectionSearchResult.iter;
						releasedConnection.swap( nextConnection );
						nextConnection.release()->deleteLater();
					}
				}
			}
		}

	private Q_SLOTS:
		void releaseNonHttpConnection()
		{
			removeAbandonedConnections( m_nonHttpConnections );
		}

	private:
		unsigned int m_maxParallelConnectionsPerTarget;
		unsigned int m_connectionTimeoutInMilliSeconds;
		std::unordered_map< QString, ConnectionList > m_connections;
		std::unordered_map< QString, ConnectionQueue > m_pendingConnections;
		ConnectionList m_nonHttpConnections;
	};

	class StandardErrorStringResolver
	{
	public:
		StandardErrorStringResolver( const Request& request, const QString& reasonPhrase )
		    : m_request{ request }
		    , m_scheme{ request.qRequest.url().scheme() }
		    , m_protocol{ StandardErrorStringResolver::protocolFromScheme( m_scheme ) }
		    , m_reasonPhrase{ reasonPhrase }
		{
		}

		QString resolve( QNetworkReply::NetworkError error ) const
		{
			if ( ! FileUtils::isFileLikeScheme( m_scheme )
			     && StandardErrorStringResolver::useStandardStatusCodeErrorString( error ) )
				return standardStatusCodeErrorString();

			const auto protocolSpecificError = protocolSpecificErrorString( error );
			if ( ! protocolSpecificError.isNull() )
				return protocolSpecificError;

			return protocolCommonErrorString( error );
		}

	private:
		static bool useStandardStatusCodeErrorString( QNetworkReply::NetworkError errorCode )
		{
			switch ( errorCode )
			{
				case QNetworkReply::UnknownContentError:               // other 4xx
				case QNetworkReply::ProtocolInvalidOperationError:     // 400
				case QNetworkReply::ContentAccessDenied:               // 403
				case QNetworkReply::ContentNotFoundError:              // 404
				case QNetworkReply::ContentOperationNotPermittedError: // 405
				case QNetworkReply::ContentConflictError:              // 409
				case QNetworkReply::ContentGoneError:                  // 410
				case QNetworkReply::UnknownServerError:                // other 5xx
				case QNetworkReply::InternalServerError:               // 500
				case QNetworkReply::OperationNotImplementedError:      // 501
				case QNetworkReply::ServiceUnavailableError:           // 503
					return true;

				default:
					return false;
			}
		}

		QString standardStatusCodeErrorString() const
		{
			return QStringLiteral( "Error transferring " ) + m_request.qRequest.url().toDisplayString()
			       + QStringLiteral( " - server replied: " ) + m_reasonPhrase;
		}

		enum class Protocol
		{
			Unknown,
			Http,
			Ftp,
			File
		};

		static Protocol protocolFromScheme( const QString& scheme )
		{
			if ( scheme == HttpUtils::httpsScheme() || scheme == HttpUtils::httpScheme() )
				return Protocol::Http;
			if ( scheme == FtpUtils::ftpsScheme() || scheme == FtpUtils::ftpScheme() )
				return Protocol::Ftp;
			if ( scheme == FileUtils::fileScheme() || scheme == FileUtils::qrcScheme() )
				return Protocol::File;

			return Protocol::Unknown;
		}

		QString protocolSpecificErrorString( QNetworkReply::NetworkError error ) const
		{
			switch ( m_protocol )
			{ // clang-format off
				case Protocol::Ftp:  return ftpErrorString( error );
				case Protocol::File: return fileErrorString( error );
				default:             return fallbackProtocolErrorString( error );
			} // clang-format on
		}

		QString ftpErrorString( QNetworkReply::NetworkError error ) const
		{
			const auto hostName = m_request.qRequest.url().host();

			switch ( error )
			{
				case QNetworkReply::ConnectionRefusedError:
					return QCoreApplication::translate( "QFtp", "Connection refused to host %1" ).arg( hostName );
				case QNetworkReply::TimeoutError:
					return QCoreApplication::translate( "QFtp", "Connection timed out to host %1" ).arg( hostName );
				case QNetworkReply::AuthenticationRequiredError:
					return QCoreApplication::translate( "QNetworkAccessFtpBackend",
					                                    "Logging in to %1 failed: authentication required" )
					    .arg( hostName );
				default:
					return {};
			}
		}

		QString fileErrorString( QNetworkReply::NetworkError error ) const
		{
			const auto scheme = m_request.qRequest.url().scheme();
			switch ( error )
			{
				case QNetworkReply::ContentOperationNotPermittedError:
					return QCoreApplication::translate( "QNetworkAccessFileBackend",
					                                    "Cannot open %1: Path is a directory" )
					    .arg( m_request.qRequest.url().toString() );
				case QNetworkReply::ProtocolUnknownError:
					return QCoreApplication::translate( "QNetworkReply", "Protocol \"%1\" is unknown" ).arg( scheme );
				case QNetworkReply::ContentAccessDenied:
				case QNetworkReply::ContentNotFoundError:
				case QNetworkReply::ProtocolFailure:
				default:
					return fileOperationErrorString( error );
			}
		}

		QString fileOperationErrorString( QNetworkReply::NetworkError error ) const
		{
			const char* const fileTranslationContext = translationContextForProtocol( Protocol::File );
			const auto unknownError = QStringLiteral( "Unknown error" );
			const auto requestUrl = m_request.qRequest.url();
			if ( error == QNetworkReply::ContentNotFoundError
			     || ( error == QNetworkReply::ContentAccessDenied
			          && m_request.operation == QNetworkAccessManager::GetOperation ) )
			{
				auto detailErrorString = QStringLiteral( "No such file or directory" );
				if ( error == QNetworkReply::ContentAccessDenied )
				{
					if ( requestUrl.scheme() == FileUtils::qrcScheme() )
						detailErrorString = unknownError;
					else
						detailErrorString = QStringLiteral( "Access denied" );
				}
				return QCoreApplication::translate( fileTranslationContext, "Error opening %1: %2" )
				    .arg( requestUrl.toString(), detailErrorString );
			}

			if ( error == QNetworkReply::ProtocolFailure )
			{
				if ( m_request.operation == QNetworkAccessManager::PutOperation )
				{
					return QCoreApplication::translate( fileTranslationContext, "Write error writing to %1: %2" )
					    .arg( requestUrl.toString(), unknownError );
				}
				return QCoreApplication::translate( fileTranslationContext, "Read error reading from %1: %2" )
				    .arg( requestUrl.toString(), unknownError );
			}

			return QCoreApplication::translate( "QIODevice", "Unknown error" );
		}

		QString fallbackProtocolErrorString( QNetworkReply::NetworkError error ) const
		{
			const char* protocolTrContext = translationContextForProtocol( m_protocol );

			switch ( error )
			{
				case QNetworkReply::ConnectionRefusedError:
					return QCoreApplication::translate( protocolTrContext, "Connection refused" );
				case QNetworkReply::TimeoutError:
					return QCoreApplication::translate( "QAbstractSocket", "Socket operation timed out" );
				case QNetworkReply::AuthenticationRequiredError: // 401
					return QCoreApplication::translate( protocolTrContext, "Host requires authentication" );
				default:
					return QString();
			}
		}

		static const char* translationContextForProtocol( Protocol protocol )
		{
			switch ( protocol )
			{
				case Protocol::Http:
					return "QHttp";
				case Protocol::Ftp:
					return "QFtp";
				case Protocol::File:
					return "QNetworkAccessFileBackend";
				default:
					return "QNetworkReply";
			}
		}

		QString protocolCommonErrorString( QNetworkReply::NetworkError error ) const
		{
			const auto hostName = m_request.qRequest.url().host();

			switch ( error )
			{ // clang-format off
				case QNetworkReply::RemoteHostClosedError:            return protocolTr( m_protocol, "Connection closed" );
				case QNetworkReply::HostNotFoundError:                return protocolTr( m_protocol, "Host %1 not found" ).arg( hostName );
				case QNetworkReply::OperationCanceledError:           return QCoreApplication::translate( "QNetworkReplyImpl", "Operation canceled" );
				case QNetworkReply::SslHandshakeFailedError:          return protocolTr( m_protocol, "SSL handshake failed" );
				case QNetworkReply::TemporaryNetworkFailureError:     return qNetworkReplyTr( "Temporary network failure." );
				case QNetworkReply::NetworkSessionFailedError:        return qNetworkReplyTr( "Network session error." );
				case QNetworkReply::BackgroundRequestNotAllowedError: return qNetworkReplyTr( "Background request not allowed." );

#if QT_VERSION >= QT_VERSION_CHECK( 5,6,0 )
				case QNetworkReply::TooManyRedirectsError:            return protocolTr( m_protocol, "Too many redirects" );
				case QNetworkReply::InsecureRedirectError:            return protocolTr( m_protocol, "Insecure redirect" );
#endif // Qt >= 5.6.0

				case QNetworkReply::ProxyConnectionRefusedError:      return qHttpSocketEngineTr( "Proxy connection refused" );
				case QNetworkReply::ProxyConnectionClosedError:       return qHttpSocketEngineTr( "Proxy connection closed prematurely" );
				case QNetworkReply::ProxyNotFoundError:               return protocolTr( m_protocol, "No suitable proxy found" );
				case QNetworkReply::ProxyTimeoutError:                return qHttpSocketEngineTr( "Proxy server connection timed out" );
				case QNetworkReply::ProxyAuthenticationRequiredError: return protocolTr( m_protocol, "Proxy requires authentication" );

				case QNetworkReply::ProtocolUnknownError:             return protocolTr( m_protocol, "Unknown protocol specified" );
				case QNetworkReply::ProtocolFailure:                  return protocolTr( m_protocol, "Data corrupted" );
				case QNetworkReply::UnknownNetworkError:              return QStringLiteral( "Unknown network error" );
				case QNetworkReply::UnknownProxyError:                return QStringLiteral( "Unknown proxy error" );

				default:
					return QCoreApplication::translate( "QIODevice", "Unknown error" );
			} // clang-format on
		}

		static QString protocolTr( Protocol protocol, const char* sourceText )
		{
			const char* protocolTrContext = StandardErrorStringResolver::translationContextForProtocol( protocol );
			return QCoreApplication::translate( protocolTrContext, sourceText );
		}

		static QString qNetworkReplyTr( const char* sourceText )
		{
			return QCoreApplication::translate( "QNetworkReply", sourceText );
		}

		static QString qHttpSocketEngineTr( const char* sourceText )
		{
			return QCoreApplication::translate( "QHttpSocketEngine", sourceText );
		}

	private:
		Request m_request;
		QString m_scheme;
		Protocol m_protocol;
		QString m_reasonPhrase;
	};

} // namespace detail


/*! Enables cloning of QNetworkReplies.
 * \since 0.11.0
 */
class CloneableNetworkReply : public QNetworkReply
{
	Q_OBJECT

public:
	/*! \return A hash of all attributes set on this reply.
	 * \since 0.11.0 moved to class CloneableNetworkReply from MockReply and changed to return a hash including the
	 * attribute values. To get just the attribute keys, use attributeKeys().
	 */
	AttributeHash attributes() const
	{
		AttributeHash result;
		for ( auto&& attribute : m_attributeSet )
			result.insert( attribute, this->attribute( attribute ) );
		return result;
	}

	/*! \return The set of all attribute keys set on this reply.
	 * \since 0.11.0
	 */
	AttributeSet attributeKeys() const
	{
		return m_attributeSet;
	}

	/*! Returns the URL of the HTTP Location header field.
	 *
	 * \return The value of the Location header field as a QUrl.
	 * \sa NetworkReplyUtils::locationHeader(const QNetworkReply*)
	 * \since 0.4.0
	 * \since 0.11.0 Move to CloneableNetworkReply from MockReply.
	 */
	QUrl locationHeader() const
	{
		return NetworkReplyUtils::locationHeader( this );
	}


protected:
	/*! Construct a new CloneableNetworkReply object.
	 *
	 * \param parent Parent QObject
	 */
	explicit CloneableNetworkReply( QObject* parent = Q_NULLPTR )
	    : QNetworkReply( parent )
	{
		setupDefaultAttributeSet();
	}

	/*! Sets an attribute for this reply.
	 * \param attribute The attribute key.
	 * \param value The value for the attribute.
	 * \sa QNetworkReply::setAttribute()
	 */
	void setAttribute( QNetworkRequest::Attribute attribute, const QVariant& value )
	{
		if ( value.isValid() )
			m_attributeSet.insert( attribute );
		else
			m_attributeSet.remove( attribute );
		QNetworkReply::setAttribute( attribute, value );
	}

	/*! Sets the attributes of this reply.
	 * \note This method clears all attributes of this reply before setting the new attribute values.
	 * \param attributes The new attributes for this reply.
	 */
	void setAttributes( const AttributeHash& attributes )
	{
		clearAttributes();
		for ( auto iter = attributes.cbegin(); iter != attributes.cend(); ++iter )
			setAttribute( iter.key(), iter.value() );
	}

	/*! Sets an attribute for this reply if the attribute is not already set.
	 *
	 * An attribute is considered to be set if it has a valid QVariant value.
	 *
	 * \param attribute The attribute key.
	 * \param value The value for the attribute.
	 * \sa setAttribute()
	 */
	void setAttributeIfNotSet( QNetworkRequest::Attribute attribute, const QVariant& value )
	{
		if ( ! this->attribute( attribute ).isValid() )
			this->setAttribute( attribute, value );
	}

	/*! Copies relevant properties from a request object.
	 *
	 * The copied properties are the request, the operation, the SSL configuration and the URL.
	 *
	 * \param request The request from which the properties are copied.
	 */
	void copyPropertiesFromRequest( const Request& request )
	{
		this->setRequest( request.qRequest );
		this->setOperation( request.operation );
		this->setSslConfiguration( request.qRequest.sslConfiguration() );
		this->updateUrl( request.qRequest );
	}

private:
	void updateUrl( const QNetworkRequest& qRequest )
	{
		auto url = qRequest.url();
		if ( FileUtils::isFileLikeScheme( url ) && url.host() == QLatin1String( "localhost" ) )
			url.setHost( QString() );
		this->setUrl( url );
	}

protected:
	/*! Copies the properties of a QNetworkReply to this CloneableNetworkReply.
	 *
	 * The copied properties include the headers and the attributes.
	 *
	 * \param source The network reply whose properties should be copied.
	 * \param additionalAttributes Set of additional attributes that will be copied from \p source.
	 * By default, only the attributes up to the ID \ref MOCKNETWORKACCESS_MAX_PREDEFINED_ATTRIBUTE_ID are copied.
	 * Use this parameter to specify additional attributes that should be copied.
	 * \sa MOCKNETWORKACCESS_MAX_PREDEFINED_ATTRIBUTE_ID
	 * \sa Manager::registerUserDefinedAttribute()
	 * \since 0.11.0
	 */
	virtual void copyReplyPropertiesFrom( const QNetworkReply* source, const AttributeSet& additionalAttributes = {} )
	{
		copyHeadersFrom( source );
		copyAttributesFrom( source, additionalAttributes );

		this->setUrl( source->url() );
		this->setOperation( source->operation() );
		this->setError( source->error(), source->errorString() );
		this->setSslConfiguration( source->sslConfiguration() );
		this->setReadBufferSize( source->readBufferSize() );
	}

	/*! Copies the attributes of another QNetworkReply.
	 * \note This method clears all attributes of this reply before setting the new attribute values.
	 * \param source The QNetworkReply from which the attributes are copied.
	 * \param additionalAttributes A set of additional attributes to copy. By default, only the attributes
	 * up to MOCKNETWORKACCESS_MAX_PREDEFINED_ATTRIBUTE_ID are copied.
	 */
	void copyAttributesFrom( const QNetworkReply* source, const AttributeSet& additionalAttributes = {} )
	{
		clearAttributes();

		auto attributes = additionalAttributes;
		for ( AttributeIdType i = 0; i < MOCKNETWORKACCESS_MAX_PREDEFINED_ATTRIBUTE_ID; ++i )
			attributes << static_cast< QNetworkRequest::Attribute >( i );
		for ( auto&& attribute : detail::asConst( attributes ) )
		{
			const auto attributeValue = source->attribute( attribute );
			if ( attributeValue.isValid() )
				this->setAttribute( attribute, attributeValue );
		}

		const auto* cloneableSource = qobject_cast< const CloneableNetworkReply* >( source );
		if ( cloneableSource )
		{
			for ( auto&& attribute : detail::asConst( cloneableSource->m_attributeSet ) )
				this->setAttribute( attribute, this->attribute( attribute ) );
		}
	}

	/*! Resets all attributes of this reply.
	 */
	void clearAttributes()
	{
		const auto attributes = m_attributeSet;
		for ( auto&& attribute : attributes )
		{
			this->setAttribute( attribute, {} );
		}
		m_attributeSet.clear();
	}

	/*! Copies the headers of a QNetworkReply to this CloneableNetworkReply.
	 *
	 * \param source The network reply whose headers should be copied.
	 * \sa copyReplyPropertiesFrom()
	 * \since 0.11.0
	 */
	void copyHeadersFrom( const QNetworkReply* source )
	{
		const auto setCookieHeader = QByteArrayLiteral( "Set-Cookie" );
		KnownHeadersSet copyKnownHeaders;

		const auto sourceRawHeaders = source->rawHeaderList();
		const auto thisRawHeaders = this->rawHeaderList();
		auto headersToBeDeleted =
#if QT_VERSION < QT_VERSION_CHECK( 5,14,0 )
		    QSet< QByteArray >::fromList( thisRawHeaders );
#else  // Qt >= 5.14.0
		    QSet< QByteArray >{ thisRawHeaders.begin(), thisRawHeaders.end() };
#endif // Qt >= 5.14.0

		for ( auto&& header : sourceRawHeaders )
		{
			headersToBeDeleted.remove( header );
			if ( header == setCookieHeader )
			{
				/* Qt doesn't properly concatenate Set-Cookie entries when returning
				 * rawHeader(). Therefore, we need to copy that header using header()
				 * (see below).
				 */
				copyKnownHeaders.insert( QNetworkRequest::SetCookieHeader );
				continue;
			}
			if ( header == HttpUtils::locationHeader() )
			{
				const auto locationHeader = NetworkReplyUtils::locationHeader( source );
				if ( locationHeader.isValid() && locationHeader.scheme().isEmpty()
				     && locationHeader == source->header( QNetworkRequest::LocationHeader ) )
				{
					/* Due to QTBUG-41061, relative location headers are not set correctly when using
					 * setRawHeader(). Therefore, we need to copy that header using header()
					 * (see below).
					 */
					copyKnownHeaders.insert( QNetworkRequest::LocationHeader );
					continue;
				}
			}
			this->setRawHeader( header, source->rawHeader( header ) );
		}

		for ( auto&& header : detail::asConst( copyKnownHeaders ) )
			this->setHeader( header, source->header( header ) );

		for ( auto&& header : detail::asConst( headersToBeDeleted ) )
			this->setRawHeader( header, {} );
	}


private:
	void setupDefaultAttributeSet()
	{
		for ( AttributeIdType i = 0; i < MOCKNETWORKACCESS_MAX_PREDEFINED_ATTRIBUTE_ID; ++i )
		{
			const auto attributeId = static_cast< QNetworkRequest::Attribute >( i );
			const auto attributeValue = QNetworkReply::attribute( attributeId );
			if ( attributeValue.isValid() )
				m_attributeSet << attributeId;
		}
	}

	AttributeSet m_attributeSet;
};

/*! Mocked QNetworkReply.
 *
 * The MockReply is used to represent the properties of a mocked reply returned by the Manager.
 * Instead of sending the request to the server and returning the reply, the MockReply returns
 * predefined (mocked) data.
 *
 * A MockReply behaves like a QNetworkReply except that it doesn't emit implementation specific signals like
 * QNetworkReplyHttpImpl::startHttpRequest() or QNetworkReplyHttpImpl::abortHttpRequest().
 */
class MockReply : public CloneableNetworkReply
{
	Q_OBJECT

	friend class MockReplyBuilder;
	friend class Rule;
	friend class NetworkReply;

public:
	/*! \return The message body of this reply. */
	QByteArray body() const
	{
		return m_body.data();
	}

	/*! \return The signal connection that is used to delay and trigger the finished() signal.
	 * If the returned signal connection is invalid, the finished() signal is not delayed.
	 * \sa SignalConnectionInfo::isValid()
	 */
	SignalConnectionInfo finishDelaySignal() const
	{
		return m_finishDelaySignal;
	}

	/*! \return \c true
	 * \sa QIODevice::isSequential()
	 */
	bool isSequential() const override
	{
		return true;
	}

	/*! \return The number of bytes available for reading.
	 *\sa QIODevice::bytesAvailable()
	 */
	qint64 bytesAvailable() const override
	{
		if ( this->isReadable() )
		{
			return m_body.bytesAvailable();
		}
		return 0;
	}

	/*! \return \c true if the reply body has been completely read.
	 * \sa QIODevice::atEnd()
	 */
	bool atEnd() const override
	{
		return m_body.atEnd();
	}

	/*! Prevents reading further body data from the reply.
	 * \sa QNetworkReply::close()
	 */
	void close() override
	{
		abort();
		QNetworkReply::close();
	}

	/*! Creates a clone of this reply.
	 *
	 * \return A new MockReply which has the same properties as this MockReply.
	 */
	virtual std::unique_ptr< MockReply > clone() const
	{
		auto clone = std::unique_ptr< MockReply >{ new MockReply() };
		clone->setBody( this->body() );
		clone->setRequest( QNetworkReply::request() );
		clone->setUrl( this->url() );
		clone->setOperation( this->operation() );
		if ( m_useStandardErrorString )
			clone->setError( this->error() );
		else
			clone->setError( this->error(), this->errorString() );
		clone->setSslConfiguration( this->sslConfiguration() );
		clone->setReadBufferSize( this->readBufferSize() );
		clone->setBehaviorFlags( this->m_behaviorFlags );

		clone->copyHeadersFrom( this );
		clone->copyAttributesFrom( this, this->attributeKeys() );

		if ( this->isOpen() )
			clone->open( this->openMode() );

		clone->setFinished( this->isFinished() );

		clone->m_finishDelaySignal = this->m_finishDelaySignal;

		return clone;
	}

	/*! Checks if this reply indicates a redirect that can be followed automatically.
	 * \return \c true if this reply's HTTP status code is valid and the status code indicates a redirect that can be
	 * followed automatically.
	 * \since 0.11.0
	 * \sa NetworkReplyUtils::isRedirectToBeFollowed()
	 * \sa BehaviorFlag::Behavior_NoAutomatic308Redirect
	 */
	bool isRedirectToBeFollowed() const
	{
		return NetworkReplyUtils::isRedirectToBeFollowed( this, m_behaviorFlags );
	}

public Q_SLOTS:
	/*! Simulates an abort of a request.
	 *
	 * \note Aborted replies finish immediately, even if a finish delay signal
	 * (MockReplyBuilder::withFinishDelayUntil()) was configured.
	 *
	 * \sa QNetworkReply::abort()
	 */
	void abort() override
	{
		if ( ! m_isAborting && this->isRunning() )
		{
			m_isAborting = true;
			if ( this->isOpen() )
				close();
			else
				Q_EMIT this->aboutToClose();
			this->setError( QNetworkReply::OperationCanceledError );
#if QT_VERSION < QT_VERSION_CHECK( 5,15,0 )
			Q_EMIT this->error( this->error() );
#else
			Q_EMIT this->errorOccurred( this->error() );
#endif
			const auto replyBodySize = m_metaDataReceived ? this->body().size() : 0;
			Q_EMIT this->downloadProgress( m_dataReceived.size(), replyBodySize );
			if ( m_behaviorFlags.testFlag( Behavior_FinalUpload00Signal ) && ! m_request.body.isEmpty() )
				Q_EMIT this->uploadProgress( 0, 0 );
			finish( false );
		}
	}


protected:
	using CloneableNetworkReply::CloneableNetworkReply;

	/*! Reads bytes from the reply's body.
	 * \param[out] data A pointer to an array where the bytes will be written to.
	 * \param maxlen The maximum number of bytes that should be read.
	 * \return The number of bytes read or -1 if an error occurred.
	 * \sa QIODevice::readData()
	 */
	qint64 readData( char* data, qint64 maxlen ) override
	{
		return m_body.read( data, maxlen );
	}

	/*! Sets the message body of this reply.
	 * \param data The body data.
	 */
	void setBody( const QByteArray& data )
	{
		m_body.setData( data );
	}

	/*! Sets the error for this reply.
	 *
	 * \param error The error code.
	 * \param errorString A human-readable string describing the error.
	 */
	void setError( QNetworkReply::NetworkError error, const QString& errorString )
	{
		m_userDefinedError = true;
		m_useStandardErrorString = false;
		QNetworkReply::setError( error, errorString );
	}

	/*! \overload
	 * This overload uses a standard error string for the given \p error code.
	 * \param error The error code to be set for this reply.
	 */
	void setError( QNetworkReply::NetworkError error )
	{
		m_userDefinedError = true;
		m_useStandardErrorString = true;
		QNetworkReply::setError( error, QString{} );
	}

	/*! \return The request for this reply.
	 */
	const Request& request() const
	{
		return m_request;
	}


	/*! Starts the simulation of the reply.
	 */
	void simulate( const Request& request, detail::Connection* connection )
	{
		m_request = request;
		m_connection = connection;
		this->copyPropertiesFromRequest( request );
		if ( this->m_finishDelaySignal.isValid() )
			this->connectFinishDelaySignal();
		this->simulateForScheme( this->url().scheme() );
	}

private:
	void connectFinishDelaySignal()
	{
		const int handleFinishDelaySignalSlotIndex = MockReply::staticMetaObject.indexOfSlot(
		    "handleFinishDelaySignal()" );
		Q_ASSERT( handleFinishDelaySignalSlotIndex != -1 );
		const auto handleFinishDelaySignalSlot = staticMetaObject.method( handleFinishDelaySignalSlotIndex );

		m_finishDelayConnection = m_finishDelaySignal.connect( this, handleFinishDelaySignalSlot );
		this->m_finishState = FinishState::WaitForFinishDelaySignal;
	}

	enum class FinishState
	{
		FinishImmediately,
		WaitForFinishDelaySignal,
		Finished
	};

private Q_SLOTS:
	void handleFinishDelaySignal()
	{
		QObject::disconnect( m_finishDelayConnection );
		this->m_finishState = FinishState::FinishImmediately;
	}

protected:
	/*! Simulates a network reply for the given scheme (protocol).
	 *
	 * Overriding this method allows implementing mocking for additional protocols.
	 *
	 * \param scheme The scheme for which this reply is simulated.
	 * The scheme significantly influences the behavior of the reply.
	 * In the regular case, when this method is called from simulate(), the \p scheme is the same as
	 * `request().qRequest.url().scheme()`.
	 *
	 * \sa simulateFileLikeReply() simulateHttpLikeReply()
	 */
	virtual void simulateForScheme( const QString& scheme )
	{
		if ( FileUtils::isFileLikeScheme( scheme ) )
			simulateFileLikeReply();
		else
			simulateHttpLikeReply();
	}

	/*! Simulates a network reply for a file-like scheme.
	 *
	 * File-like means `file:`, `qrc:` or `assets:` (Android) scheme.
	 *
	 * \sa FileUtils::isFileLikeScheme()
	 */
	void simulateFileLikeReply()
	{
		this->setAttributeIfNotSet( QNetworkRequest::ConnectionEncryptedAttribute, false );

		switch ( m_request.operation )
		{
			case QNetworkAccessManager::GetOperation:
			case QNetworkAccessManager::HeadOperation:
				this->simulateFileLikeGetOrHeadRequest();
				break;
			case QNetworkAccessManager::PutOperation:
				if ( this->url().scheme() == FileUtils::qrcScheme() )
					this->setAccessDeniedErrorForQrcPutReply( m_request );
				this->simulateFileLikePutRequest();
				break;
			default:
				this->simulateFileLikeRequestWithProtocolError();
				break;
		}

		this->openIODeviceForReading();
		this->updateErrorString( m_request );

		simulateFinish();
	}

	/*! Simulates a network reply for a HTTP-like scheme.
	 *
	 * HTTP-like means another not file-like (see simulateFileLikeReply()). Effectively, this means `http:` and `ftp:`
	 * scheme.
	 */
	void simulateHttpLikeReply()
	{
		simulateConnection();
	}

private Q_SLOTS:
	void simulateConnection()
	{
		const auto isEncrypted = this->setEncryptedAttribute();
		if ( isEncrypted && m_connection->isNew() )
			QMetaObject::invokeMethod( this, "encrypted", Qt::QueuedConnection );

		QMetaObject::invokeMethod( this, "handleConnectionSimulated", Qt::QueuedConnection );
	}

private:
	bool setEncryptedAttribute()
	{
		const auto scheme = this->url().scheme().toLower();
		const bool isEncrypted = scheme == HttpUtils::httpsScheme();
		this->setAttribute( QNetworkRequest::ConnectionEncryptedAttribute, QVariant::fromValue( isEncrypted ) );
		return isEncrypted;
	}

private Q_SLOTS:
	void handleConnectionSimulated()
	{
		if ( m_isAborting )
			return;

		if ( ! m_request.body.isNull() )
		{
			simulateUpload();
			return;
		}

		simulateResponse();
	}

	void simulateUpload()
	{
		if ( m_isAborting )
			return; // LCOV_EXCL_LINE - Safety measure. Should not happen with the current implementation.

		m_didUpload = true;
		this->emitUploadProgressSignal( m_request );

		QMetaObject::invokeMethod( this, "handleUploadSimulated", Qt::QueuedConnection );
	}

private:
	void emitUploadProgressSignal( const Request& request )
	{
		this->emitUploadProgressSignal( request.body.size(), request.body.size() );
	}

	void emitUploadProgressSignal( qint64 sent, qint64 total )
	{
		QMetaObject::invokeMethod( this,
		                           "uploadProgress",
		                           Qt::QueuedConnection,
		                           Q_ARG( qint64, sent ),
		                           Q_ARG( qint64, total ) );
	}

private Q_SLOTS:
	void handleUploadSimulated()
	{
		if ( m_isAborting )
			return;

		QMetaObject::invokeMethod( this, "simulateResponse", Qt::QueuedConnection );
	}

	void simulateResponse()
	{
		if ( m_isAborting )
			return; // LCOV_EXCL_LINE - Safety measure. Should not happen with the current implementation.

		m_metaDataReceived = true;
		this->updateHttpStatusCode();
		this->updateHttpReasonPhrase();
		this->updateContentTypeHeader();
		this->updateContentLengthHeader();
		this->updateRedirectionTargetAttribute();
		this->updateErrorString( m_request );
		this->setHttpReplyAttributes();
		this->openIODeviceForReading();

		QMetaObject::invokeMethod( this, "metaDataChanged", Qt::QueuedConnection );

		if ( ! this->body().isNull() )
		{
			QMetaObject::invokeMethod( this, "simulateDownload", Qt::QueuedConnection );
			return;
		}

		emitErrorSignalIfError();

		QMetaObject::invokeMethod( this, "handleDownloadSimulated", Qt::QueuedConnection );
	}

private:
	void emitErrorSignalIfError()
	{
		if ( this->error() != QNetworkReply::NoError )
			emitErrorSignal();
	}

	void emitErrorSignal()
	{
#if QT_VERSION < QT_VERSION_CHECK( 5,15,0 )
		QMetaObject::invokeMethod( this,
		                           "error",
		                           Qt::QueuedConnection,
		                           Q_ARG( QNetworkReply::NetworkError, this->error() ) );
#else
		QMetaObject::invokeMethod( this,
		                           "errorOccurred",
		                           Qt::QueuedConnection,
		                           Q_ARG( QNetworkReply::NetworkError, this->error() ) );
#endif
	}

private Q_SLOTS:
	void simulateDownload()
	{
		if ( m_isAborting )
			return;

		const auto replyBodySize = this->body().size();
		m_dataReceived.setData( this->body() ); // "instant download"
		if ( replyBodySize > 0 )
		{
			QMetaObject::invokeMethod( this, "readyRead", Qt::QueuedConnection );
			this->emitDownloadProgressSignal( m_dataReceived.size(), replyBodySize );
		}
		emitErrorSignalIfError();
		QMetaObject::invokeMethod( this, "handleDownloadSimulated", Qt::QueuedConnection );
	}

private:
	void emitDownloadProgressSignal( qint64 received, qint64 total )
	{
		QMetaObject::invokeMethod( this,
		                           "downloadProgress",
		                           Qt::QueuedConnection,
		                           Q_ARG( qint64, received ),
		                           Q_ARG( qint64, total ) );
	}

private Q_SLOTS:
	void handleDownloadSimulated()
	{
		if ( m_isAborting )
			return;

		const auto replyBodySize = this->body().size();
		this->emitDownloadProgressSignal( m_dataReceived.size(), replyBodySize );
		if ( m_behaviorFlags.testFlag( Behavior_FinalUpload00Signal ) && m_didUpload )
			this->emitUploadProgressSignal( 0, 0 );

		simulateFinish();
	}

private:
	void simulateFinish()
	{
		if ( m_isAborting )
			return; // LCOV_EXCL_LINE - Safety measure. Should not happen with the current implementation.

		switch ( this->m_finishState )
		{
			case FinishState::FinishImmediately:
				finish();
				break;
			case FinishState::WaitForFinishDelaySignal:
				Q_ASSERT( this->m_finishDelaySignal.isValid() );
				connectFinishDelaySignalToFinish();
				break;
			// LCOV_EXCL_START
			case FinishState::Finished:
				Q_ASSERT_X( false, Q_FUNC_INFO, "simulateFinished() called on an already finished MockReply" );
				return;
				// LCOV_EXCL_STOP
		}
	}

	void connectFinishDelaySignalToFinish()
	{
		QObject::disconnect( m_finishDelayConnection );

		const auto finishSlotIndex = MockReply::staticMetaObject.indexOfSlot( "finish()" );
		Q_ASSERT( finishSlotIndex != -1 );
		const auto finishSlot = staticMetaObject.method( finishSlotIndex );

		m_finishDelayConnection = m_finishDelaySignal.connect( this, finishSlot );
	}

private Q_SLOTS:
	void finish( bool queued = true )
	{
		QObject::disconnect( m_finishDelayConnection );

		this->m_finishState = FinishState::Finished;
		this->setFinished( true );
		this->emitFinishedSignals( queued );
	}

private:
	void setAccessDeniedErrorForQrcPutReply( const Request& request )
	{
		if ( m_userDefinedError )
		{
			if ( this->error() == QNetworkReply::ContentAccessDenied )
				return;

			qCWarning( log ) << "Reply was configured to reply with error" << this->error()
			                 << "but a qrc request does not support writing operations and therefore has to reply with"
			                 << QNetworkReply::ContentAccessDenied << ". Overriding configured behavior.";
		}

		const auto errorString = detail::StandardErrorStringResolver( request, this->reasonPhrase() )
		                             .resolve( QNetworkReply::ContentAccessDenied );
		QNetworkReply::setError( QNetworkReply::ContentAccessDenied, errorString );
	}

	QString reasonPhrase() const
	{
		return this->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).toString();
	}

	void setProtocolUnknownError( const Request& request )
	{
		if ( m_userDefinedError )
		{
			if ( this->error() == QNetworkReply::ProtocolUnknownError )
				return;

			if ( FileUtils::isFileLikeScheme( request.qRequest.url() ) )
			{
				qCWarning( log ) << "Reply was configured to reply with error" << this->error() << "but a request"
				                 << request.verb().toUtf8().constData()
				                 << request.qRequest.url().toString().toUtf8().constData() << "must be replied with"
				                 << QNetworkReply::ProtocolUnknownError << ". Overriding configured behavior.";
			}
		}

		const auto errorString = detail::StandardErrorStringResolver( request, this->reasonPhrase() )
		                             .resolve( QNetworkReply::ProtocolUnknownError );
		QNetworkReply::setError( QNetworkReply::ProtocolUnknownError, errorString );
	}

	void setHttpReplyAttributes()
	{
		if ( this->attribute( QNetworkRequest::HttpStatusCodeAttribute ).isValid() )
		{
			this->setAttributeIfNotSet( QNetworkRequest::HttpPipeliningWasUsedAttribute, false );
#if QT_VERSION >= QT_VERSION_CHECK( 5,3,0 ) && QT_VERSION < QT_VERSION_CHECK( 6,0,0 )
			this->setAttributeIfNotSet( QNetworkRequest::SpdyWasUsedAttribute, false );
#endif
#if QT_VERSION >= QT_VERSION_CHECK( 5,15,0 )
			this->setAttributeIfNotSet( QNetworkRequest::Http2WasUsedAttribute, false );
#elif QT_VERSION >= QT_VERSION_CHECK( 5,9,0 )
			this->setAttributeIfNotSet( QNetworkRequest::HTTP2WasUsedAttribute, false );
#endif // Qt >= 5.9.0
		}
	}


	void updateContentTypeHeader()
	{
		if ( ! this->header( QNetworkRequest::ContentTypeHeader ).isValid() //
		     && ! this->body().isEmpty() )
		{
			const auto mimeType = guessMimeType( this->url(), m_body.data() );
			this->setHeader( QNetworkRequest::ContentTypeHeader, QVariant::fromValue( mimeType.name() ) );
		}
	}

	void updateContentLengthHeader()
	{
		if ( this->rawHeader( "Transfer-Encoding" ).isEmpty()                    //
		     && ! this->header( QNetworkRequest::ContentLengthHeader ).isValid() //
		     && ! this->body().isNull() )
		{
			this->setHeader( QNetworkRequest::ContentLengthHeader, QVariant::fromValue( this->body().length() ) );
		}
	}

	void updateHttpStatusCode()
	{
		auto statusCodeAttr = this->attribute( QNetworkRequest::HttpStatusCodeAttribute );
		if ( statusCodeAttr.isValid() )
		{
			bool canConvertToInt = false;
			statusCodeAttr.toInt( &canConvertToInt );
			if ( ! canConvertToInt )
			{
				qCWarning( log ) << "Invalid type for HttpStatusCodeAttribute. Must be int but was:"
				                 << statusCodeAttr.typeName();
				statusCodeAttr = QVariant();
				this->setAttribute( QNetworkRequest::HttpStatusCodeAttribute, statusCodeAttr );
				/* Invalid status code results in QNetworkReply::RemoteHostClosedError
				 * See `QHttpProtocolHandler::_q_receiveReply()`
				 * (qtbase\src\network\access\qhttpprotocolhandler.cpp, line ~103-106)
				 */
				const auto errorString = detail::StandardErrorStringResolver( m_request, this->reasonPhrase() )
				                             .resolve( QNetworkReply::RemoteHostClosedError );
				QNetworkReply::setError( QNetworkReply::RemoteHostClosedError, errorString );
			}
		}
		else
		{
			const int statusCode = HttpStatus::networkErrorToStatusCode( this->error() );
			if ( statusCode > 0 )
			{
				statusCodeAttr = QVariant::fromValue( statusCode );
				this->setAttribute( QNetworkRequest::HttpStatusCodeAttribute, statusCodeAttr );
			}
		}
	}

	void updateHttpReasonPhrase()
	{
		const auto statusCodeAttr = this->attribute( QNetworkRequest::HttpStatusCodeAttribute );
		if ( ! this->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).isValid() && statusCodeAttr.isValid() )
		{
			// Set standard reason phrase
			this->setAttribute( QNetworkRequest::HttpReasonPhraseAttribute,
			                    HttpStatus::reasonPhrase( statusCodeAttr.toInt() ).toUtf8() );
		}
	}

	void updateRedirectionTargetAttribute()
	{
		/* Qt doesn't set the RedirectionTargetAttribute for UseProxy (305) redirects.
		 * See QNetworkReplyHttpImplPrivate::checkForRedirect(const int statusCode)
		 */
		const auto statusCodeAttr = this->attribute( QNetworkRequest::HttpStatusCodeAttribute );
		if ( this->isRedirectToBeFollowed() && statusCodeAttr.toInt() != static_cast< int >( HttpStatus::UseProxy ) )
		{
			const auto locationHeaderUrl = this->locationHeader();
			if ( ! locationHeaderUrl.isValid() )
				return;
			this->setAttribute( QNetworkRequest::RedirectionTargetAttribute, locationHeaderUrl );
		}
	}

	void updateErrorString( const Request& request )
	{
		if ( m_useStandardErrorString )
		{
			const auto errorString = detail::StandardErrorStringResolver( request, this->reasonPhrase() )
			                             .resolve( this->error() );
			this->setError( this->error(), errorString );
		}
	}

protected Q_SLOTS:

	/*! Tunes the behavior of this MockReply.
	 *
	 * \param behaviorFlags Combination of BehaviorFlags to define some details of this MockReply's behavior.
	 * \note Only certain BehaviorFlags have an effect on a MockReply.
	 * \sa BehaviorFlag
	 */
	void setBehaviorFlags( BehaviorFlags behaviorFlags )
	{
		m_behaviorFlags = behaviorFlags;
	}


private:
	void openIODeviceForReading()
	{
		m_body.open( QIODevice::ReadOnly );
		this->setOpenMode( QIODevice::ReadOnly );
	}

	void simulateFileLikeRequestWithProtocolError()
	{
		this->setProtocolUnknownError( m_request );
		this->emitErrorSignalIfError();
		this->emitDownloadProgressSignal( 0, 0 );
		if ( m_behaviorFlags.testFlag( Behavior_FinalUpload00Signal ) )
			this->emitUploadProgressSignal( 0, 0 );
	}

	void simulateFileLikePutRequest()
	{
		m_body.setData( QByteArray() );

		this->emitErrorSignalIfError();
		const bool hasError = this->error() != QNetworkReply::NoError;
		if ( ! hasError && ! m_request.body.isEmpty() )
		{
			this->emitUploadProgressSignal( m_request );
			this->emitDownloadProgressSignal( 0, 0 );
		}
		else
		{
			this->emitDownloadProgressSignal( 0, 0 );
			this->emitUploadProgressSignal( 0, 0 );
		}
	}

	void simulateFileLikeGetOrHeadRequest()
	{
		this->updateContentLengthHeader();
		if ( this->error() != QNetworkReply::NoError )
		{
			this->emitErrorSignal();
			return;
		}

		QMetaObject::invokeMethod( this, "metaDataChanged", Qt::QueuedConnection );
		this->emitDownloadProgressSignal( m_body.size(), m_body.size() );
		QMetaObject::invokeMethod( this, "readyRead", Qt::QueuedConnection );
	}

	void emitFinishedSignals( bool queued = true )
	{
		const auto connectionType = queued ? Qt::QueuedConnection : Qt::DirectConnection;
		if ( emitsReadChannelFinishedSignals() )
			QMetaObject::invokeMethod( this, "readChannelFinished", connectionType );
		QMetaObject::invokeMethod( this, "finished", connectionType );
	}

	bool emitsReadChannelFinishedSignals() const
	{
		if ( ! FileUtils::isFileLikeScheme( this->url() ) )
			return true;

		if ( m_request.operation == QNetworkAccessManager::PutOperation
		     || this->error() == QNetworkReply::ProtocolUnknownError )
			return true;

		return false;
	}


	QBuffer m_body;
	BehaviorFlags m_behaviorFlags = Behavior_Expected;
	bool m_userDefinedError = false;
	bool m_useStandardErrorString = true;
	Request m_request;
	detail::Connection* m_connection = nullptr;
	bool m_metaDataReceived = false;
	bool m_didUpload = false;
	bool m_isAborting = false;
	QBuffer m_dataReceived;
	FinishState m_finishState = FinishState::FinishImmediately;
	SignalConnectionInfo m_finishDelaySignal;
	QMetaObject::Connection m_finishDelayConnection;
};


/*! Creates MockReply objects with predefined properties.
 *
 * This class is a configurable factory for MockReply objects.
 * The \c with*() methods configure the properties of the created replies.
 * To create a reply according to the configured properties, call createReply().
 *
 * Similar to the Rule class, the MockReplyBuilder implements a chainable interface for the configuration.
 */
class MockReplyBuilder
{
public:
	/*! Creates an unconfigured MockReplyBuilder.
	 *
	 * \note Calling createReply() on an unconfigured MockReplyBuilder will return a \c Q_NULLPTR.
	 */
	MockReplyBuilder() = default;

	/*! Creates a MockReplyBuilder by copying another one.
	 * \param other The MockReplyBuilder which is being copied.
	 */
	MockReplyBuilder( const MockReplyBuilder& other )
	{
		if ( other.m_replyPrototype )
			m_replyPrototype = other.m_replyPrototype->clone();
		m_userDefinedError = other.m_userDefinedError;
	}

	/*! Creates a MockReplyBuilder by moving another one.
	 * \param other The MockReplyBuilder which is being moved.
	 */
	MockReplyBuilder( MockReplyBuilder&& other ) noexcept = default;

	/*! Destroys this MockReplyBuilder.
	 */
	~MockReplyBuilder() = default;

	/*! Swaps this MockReplyBuilder with another one.
	 * \param other The MockReplyBuilder to be exchanged with this one.
	 */
	void swap( MockReplyBuilder& other ) noexcept
	{
		if ( this != &other )
		{
			m_replyPrototype.swap( other.m_replyPrototype );
			std::swap( m_userDefinedError, other.m_userDefinedError );
		}
	}

	/*! Swaps two MockReplyBuilders.
	 * \param left One MockReplyBuilder to be exchanged.
	 * \param right The other MockReplyBuilder to be exchanged.
	 */
	friend void swap( MockReplyBuilder& left, MockReplyBuilder& right ) noexcept
	{
		left.swap( right );
	}

	/*! Configures this MockReplyBuilder identical to another one.
	 * \param other The MockReplyBuilder whose configuration is being copied.
	 * \return \c this
	 */
	MockReplyBuilder& operator=( const MockReplyBuilder& other )
	{
		if ( this != &other )
		{
			if ( other.m_replyPrototype )
				m_replyPrototype = other.m_replyPrototype->clone();
			else
				m_replyPrototype.reset();
			m_userDefinedError = other.m_userDefinedError;
		}
		return *this;
	}

	/*! Configures this MockReplyBuilder identical to another one by moving the other one.
	 * \param other The MockReplyBuilder which is being moved.
	 * \return \c this
	 */
	MockReplyBuilder& operator=( MockReplyBuilder&& other ) noexcept = default;

	/*! Compares two MockReplyBuilders for equality.
	 * \param left One MockReplyBuilder to be compared.
	 * \param right The other MockReplyBuilder to be compared.
	 * \return \c true if \p left and \p right have the same properties configured
	 * and thus create equal MockReply objects.
	 */
	friend bool operator==( const MockReplyBuilder& left, const MockReplyBuilder& right )
	{
		if ( &left == &right )
			return true;

		const auto* leftReply = left.m_replyPrototype.get();
		const auto* rightReply = right.m_replyPrototype.get();

		if ( leftReply == rightReply )
			return true;

		if ( ! leftReply || ! rightReply )
			return false;

		// clang-format off
		if (   leftReply->body()              != rightReply->body()
		    || leftReply->rawHeaderPairs()    != rightReply->rawHeaderPairs()
		    || leftReply->attributes()        != rightReply->attributes()
		    || leftReply->error()             != rightReply->error()
		    || leftReply->errorString()       != rightReply->errorString()
		    || leftReply->finishDelaySignal() != rightReply->finishDelaySignal() )
			return false;
		// clang-format on

		return true;
	}

	/*! Compares two MockReplyBuilders for inequality.
	 * \param left One MockReplyBuilder to be compared.
	 * \param right The other MockReplyBuilder to be compared.
	 * \return \c true if \p left and \p right have different properties configured
	 * and thus create different MockReply objects.
	 */
	friend bool operator!=( const MockReplyBuilder& left, const MockReplyBuilder& right )
	{
		return ! ( left == right );
	}

	/*! Configures this MockReplyBuilder identical to another one.
	 * This method is identical to the copy operator and exists just to provide a consistent, chainable interface.
	 * \param other The MockReplyBuilder which is being copied.
	 * \return A reference to this %MockReplyBuilder.
	 */
	MockReplyBuilder& with( const MockReplyBuilder& other )
	{
		return ( *this = other );
	}

	/*! Configures this MockReplyBuilder identical to another one by moving the other one.
	 *
	 * This method is identical to the move operator and exists just to provide a consistent, chainable interface.
	 *
	 * \param other The MockReplyBuilder which is being moved.
	 * \return A reference to this %MockReplyBuilder.
	 */
	MockReplyBuilder& with( MockReplyBuilder&& other ) noexcept
	{
		return ( *this = std::move( other ) );
	}

	/*! Sets the body for the replies.
	 * \param data The data used as the message body for the replies.
	 * \param contentType The media type of \p data. This is used to set the `Content-Type` header as defined by
	 * [RFC 7231, section 3.1.1.5](https://www.rfc-editor.org/rfc/rfc7231#section-3.1.1.5). If the provided `QString` is
	 * null, the `Content-Type` header is not changed.
	 * \since 0.11.0 added the \p contentType parameter.
	 * \return A reference to this %MockReplyBuilder.
	 */
	MockReplyBuilder& withBody( const QByteArray& data, const QString& contentType = {} )
	{
		auto* proto = ensureReplyPrototype();
		proto->setBody( data );
		if ( ! contentType.isNull() )
		{
			proto->setHeader( QNetworkRequest::ContentTypeHeader, QVariant::fromValue( contentType.toLatin1() ) );
		}
		return *this;
	}

	/*! Sets the body for the replies to a JSON document.
	 * \param json The data used as the message body for the replies.
	 * \return A reference to this %MockReplyBuilder.
	 */
	MockReplyBuilder& withBody( const QJsonDocument& json )
	{
		auto* proto = ensureReplyPrototype();
		proto->setBody( json.toJson( QJsonDocument::Compact ) );
		proto->setHeader( QNetworkRequest::ContentTypeHeader,
		                  QVariant::fromValue( QStringLiteral( "application/json" ) ) );
		return *this;
	}

	/*! Sets the body for the replies to the content of a file.
	 *
	 * The file needs to exist at the time this method is called because the file's
	 * content is read and stored in this MockReplyBuilder by this method.
	 *
	 * This method also tries to determine the file's MIME type using
	 * QMimeDatabase::mimeTypeForFileNameAndData() and sets
	 * the [QNetworkRequest::ContentTypeHeader] accordingly.
	 * If this does not determine the MIME type correctly or if you want to set the
	 * MIME type explicitly, use withHeader() or withRawHeader() *after* calling this method.
	 *
	 * \param filePath The path to the file whose content is used as the message body for the replies.
	 * \return A reference to this %MockReplyBuilder.
	 * \sa [QNetworkRequest::ContentTypeHeader]
	 * \sa withHeader()
	 * [QNetworkRequest::ContentTypeHeader]: http://doc.qt.io/qt-5/qnetworkrequest.html#KnownHeaders-enum
	 */
	MockReplyBuilder& withFile( const QString& filePath )
	{
		auto* proto = ensureReplyPrototype();

		QFile file( filePath );
		if ( file.open( QIODevice::ReadOnly ) )
		{
			const auto data = file.readAll();
			file.close();
			proto->setBody( data );
			const auto mimeType = QMimeDatabase().mimeTypeForFileNameAndData( filePath, data );
			proto->setHeader( QNetworkRequest::ContentTypeHeader, QVariant::fromValue( mimeType.name() ) );
		}
		return *this;
	}

	/*! Sets the status code and reason phrase for the replies.
	 *
	 * \note \parblock
	 * If the \p statusCode is an error code, this will also set the corresponding QNetworkReply::NetworkError unless
	 * it was already set using withError(). If no error string is set explicitly, a standard error string based on
	 * the reason phrase will be set by the Manager before returning the reply.
	 * \endparblock
	 *
	 * \param statusCode The HTTP status code.
	 * \param reasonPhrase The HTTP reason phrase. If it is a null QString(), the standard reason phrase for the
	 * \p statusCode will be used, if available and unless a reason phrase was already set.
	 * \return A reference to this %MockReplyBuilder.
	 *
	 * \sa withError()
	 */
	MockReplyBuilder& withStatus( int statusCode = static_cast< int >( HttpStatus::OK ), const QString& reasonPhrase = {} )
	{
		auto* proto = ensureReplyPrototype();
		proto->setAttribute( QNetworkRequest::HttpStatusCodeAttribute, QVariant::fromValue( statusCode ) );

		auto phrase = reasonPhrase;
		if ( phrase.isNull() )
			phrase = HttpStatus::reasonPhrase( statusCode );
		proto->setAttribute( QNetworkRequest::HttpReasonPhraseAttribute, QVariant::fromValue( phrase.toUtf8() ) );

		if ( ! m_userDefinedError )
		{
			proto->setError( HttpStatus::statusCodeToNetworkError( statusCode ) );
			proto->m_userDefinedError = false; // need to set it back because it is set inside setError()
		}
		checkErrorAndStatusCodeConsistency();

		return *this;
	}

	/*! Sets a header for the replies.
	 *
	 * Calling this method with the same header again will override the previous value.
	 *
	 * \param header The header.
	 * \param value The value for the header.
	 * \return A reference to this %MockReplyBuilder.
	 *
	 * \sa QNetworkReply::setHeader()
	 */
	MockReplyBuilder& withHeader( QNetworkRequest::KnownHeaders header, const QVariant& value )
	{
		ensureReplyPrototype()->setHeader( header, value );
		return *this;
	}

	/*! Sets a raw header for the replies.
	 *
	 * Calling this method with the same header again will override the previous value.
	 * To add multiple header values for the same header, concatenate the values
	 * separated by comma. A notable exception from this rule is the \c Set-Cookie
	 * header which should be separated by newlines (`\\n`).
	 *
	 * \param header The header.
	 * \param value The value for the header.
	 * \return A reference to this %MockReplyBuilder.
	 *
	 * \sa QNetworkReply::setRawHeader()
	 */
	MockReplyBuilder& withRawHeader( const QByteArray& header, const QByteArray& value )
	{
		ensureReplyPrototype()->setRawHeader( header, value );
		return *this;
	}

	/*! Sets an attribute for the replies.
	 *
	 * Calling this method with the same attribute again will override the previous value.
	 *
	 * \param attribute The attribute.
	 * \param value The value for the attribute.
	 * \return A reference to this %MockReplyBuilder.
	 */
	MockReplyBuilder& withAttribute( QNetworkRequest::Attribute attribute, const QVariant& value )
	{
		ensureReplyPrototype()->setAttribute( attribute, value );
		return *this;
	}

	/*! Sets the error for the replies.
	 *
	 * \note \parblock
	 * If the \p error corresponds to a known HTTP status code, the reply returned by the Manager will have the
	 * corresponding HTTP status code attribute set if no status code was set explicitly (see withStatus()).\n
	 * If both the error code and the HTTP status code are set and they do not match, a warning is issued because this
	 * is a state which cannot happen with a real QNetworkReply.
	 *
	 * When using this overload, the error string will be set to a standard error string based on the reason phrase when
	 * the reply is returned from the Manager. To set a different error string, use the other overload
	 * withError( QNetworkReply::NetworkError, const QString& ).
	 *
	 * Note that both the automatic setting of the HTTP status code and the error string are not reflected by the
	 * MockReply returned by createReply(). Both things are handled by the Manager class and therefore are only
	 * reflected by the replies returned from a Manager instance.
	 * \endparblock
	 *
	 * \param error The [QNetworkReply::NetworkError] code.
	 * \return A reference to this %MockReplyBuilder.
	 *
	 * \sa withStatus()
	 * \sa withError( QNetworkReply::NetworkError, const QString& )
	 * [QNetworkReply::NetworkError]: https://doc.qt.io/qt-5/qnetworkreply.html#NetworkError-enum
	 */
	MockReplyBuilder& withError( QNetworkReply::NetworkError error )
	{
		m_userDefinedError = true;
		ensureReplyPrototype()->setError( error );

		checkErrorAndStatusCodeConsistency();
		return *this;
	}

	/*! Sets the error and error string for the replies.
	 *
	 * \note In many cases, it is neither necessary nor desirable to set the error string for the reply explicitly. The
	 * Manager sets suitable standard error strings for error codes when using withError( QNetworkReply::NetworkError ).
	 * However, there can be cases where the standard error strings do not match those of a real QNetworkAccessManager
	 * (for example when a custom network access manager is used). In such cases, this overload allows setting an
	 * explicit error string.
	 *
	 * \param error The [QNetworkReply::NetworkError] code.
	 * \param errorString A message used as error string (see QNetworkReply::errorString()).
	 * \return A reference to this %MockReplyBuilder.
	 * [QNetworkReply::NetworkError]: https://doc.qt.io/qt-5/qnetworkreply.html#NetworkError-enum
	 *
	 * \sa withError( QNetworkReply::NetworkError )
	 */
	MockReplyBuilder& withError( QNetworkReply::NetworkError error, const QString& errorString )
	{
		m_userDefinedError = true;
		ensureReplyPrototype()->setError( error, errorString );
		checkErrorAndStatusCodeConsistency();
		return *this;
	}

	/*! Convenience method to configure redirection for the replies.
	 *
	 * This sets the [QNetworkRequest::LocationHeader] and the HTTP status code.
	 * \note Due to QTBUG-41061, the [QNetworkRequest::LocationHeader] returned by QNetworkReply::header() will be an
	 * empty (invalid) URL when \p targetUrl is relative. The redirection will still work as expected.
	 * QNetworkReply::rawHeader() always returns the correct value for the Location header.
	 *
	 * \param targetUrl The URL of the redirection target. Can be relative or absolute.
	 * If it is relative, it will be made absolute using the URL of the requests that matched the Rule as base.
	 * Note that the _targetUrl_ has to be a valid URL. Else the Location header will be set to an empty value which
	 * behaves like a redirect to the current URL. To set the Location header to an invalid URL, set it using
	 * withRawHeader().
	 * \param statusCode The HTTP status code to be used. Should normally be in the 3xx range.
	 * \return A reference to this %MockReplyBuilder.
	 * \sa https://bugreports.qt.io/browse/QTBUG-41061
	 * \sa withRawHeader()
	 * \sa withStatus()
	 * [QNetworkRequest::LocationHeader]: http://doc.qt.io/qt-5/qnetworkrequest.html#KnownHeaders-enum
	 */
	MockReplyBuilder& withRedirect( const QUrl& targetUrl, HttpStatus::Code statusCode = HttpStatus::Found )
	{
		ensureReplyPrototype()->setRawHeader( HttpUtils::locationHeader(), targetUrl.toEncoded() );
		withStatus( static_cast< int >( statusCode ) );
		return *this;
	}

	/*! Adds an HTTP authentication challenge to the replies and sets their HTTP status code to 401 (Unauthorized).
	 *
	 * \param authChallenge The authentication challenge to be added to the replies. Must be a valid Challenge or
	 * this method does not add the authentication challenge.
	 * \return A reference to this %MockReplyBuilder.
	 *
	 * \sa HttpUtils::Authentication::Challenge::isValid()
	 * \sa QNetworkReply::setRawHeader()
	 */
	MockReplyBuilder& withAuthenticate( const HttpUtils::Authentication::Challenge::Ptr& authChallenge )
	{
		MockReply* proto = ensureReplyPrototype();
		if ( authChallenge && authChallenge->isValid() )
		{
			proto->setRawHeader( HttpUtils::wwwAuthenticateHeader(), authChallenge->authenticateHeader() );
			withStatus( static_cast< int >( HttpStatus::Unauthorized ) );
		}
		return *this;
	}

	/*! Adds an HTTP Basic authentication challenge to the replies and sets their HTTP status code to
	 * 401 (Unauthorized).
	 *
	 * \param realm The realm to be used for the authentication challenge.
	 * \return A reference to this %MockReplyBuilder.
	 *
	 * \sa withAuthenticate(const HttpUtils::Authentication::Challenge::Ptr&)
	 */
	MockReplyBuilder& withAuthenticate( const QString& realm )
	{
		HttpUtils::Authentication::Challenge::Ptr authChallenge( new HttpUtils::Authentication::Basic( realm ) );
		return withAuthenticate( authChallenge );
	}

	/*! Adds a cookie to the replies.
	 *
	 * \note \parblock
	 * - The cookie will be appended to the current list of cookies.
	 * To replace the complete list of cookies, use withHeader() and set the
	 * [QNetworkRequest::SetCookieHeader] to a QList<QNetworkCookie>.
	 * - This method does *not* check if a cookie with the same name already
	 * exists in the [QNetworkRequest::SetCookieHeader].
	 * RFC 6265 says that replies SHOULD NOT contain multiple cookies with the
	 * same name. However, to allow simulating misbehaving servers, this method
	 * still allows this.
	 * \endparblock
	 *
	 * \param cookie The cookie to be added to the replies.
	 * \return A reference to this %MockReplyBuilder.
	 *
	 * \sa [QNetworkRequest::SetCookieHeader]
	 * [QNetworkRequest::SetCookieHeader]: http://doc.qt.io/qt-5/qnetworkrequest.html#KnownHeaders-enum
	 */
	MockReplyBuilder& withCookie( const QNetworkCookie& cookie )
	{
		MockReply* proto = ensureReplyPrototype();
		QList< QNetworkCookie > cookies = proto->header( QNetworkRequest::SetCookieHeader )
		                                      .value< QList< QNetworkCookie > >();
		cookies.append( cookie );
		proto->setHeader( QNetworkRequest::SetCookieHeader, QVariant::fromValue( cookies ) );
		return *this;
	}


	/*! Adds a delay before the QNetworkReply::finished() signal is emitted.
	 *
	 * The `finished()` signal of the replies is delay until a given signal is emitted.
	 *
	 * \note It is important that the given signal is emitted **after** the reply was returned
	 * from the manager. If the signal is emitted before the reply is returned from the manager, the reply will
	 * never emit the `finished()` signal.
	 *
	 * \note An aborted MockedReply will finish immediately, even if a finish delay was configured.
	 *
	 * \param sender The QObject which emits the signal to wait for with the `finished()` signal.
	 * \param signalSignature The signature of the signal to wait for. Note that this should be given **without** using
	 * the SIGNAL() macro. So for example simply `builder.withInitialDelayUntil( someObject, "someSignal()" )`.
	 * \param connectionType The type of the connection.
	 * \return A reference to this %MockReplyBuilder.
	 *
	 */
	MockReplyBuilder& withFinishDelayUntil( QObject* sender,
	                                        const char* signalSignature,
	                                        Qt::ConnectionType connectionType = Qt::AutoConnection )
	{
		Q_ASSERT( sender );
		const int signalIndex = sender->metaObject()->indexOfSignal(
		    QMetaObject::normalizedSignature( signalSignature ).constData() );
		Q_ASSERT( signalIndex != -1 );
		return withFinishDelayUntil( sender, sender->metaObject()->method( signalIndex ), connectionType );
	}

	/*! \overload
	 *
	 * \param sender The QObject which emits the signal to wait for with the `finished()` signal.
	 * \param metaSignal The QMetaMethod of the signal.
	 * \param connectionType The type of the connection.
	 * \return A reference to this %MockReplyBuilder.
	 */
	MockReplyBuilder& withFinishDelayUntil( QObject* sender,
	                                        const QMetaMethod& metaSignal,
	                                        Qt::ConnectionType connectionType = Qt::AutoConnection )
	{
		SignalConnectionInfo signalConnection( sender, metaSignal, connectionType );
		Q_ASSERT( signalConnection.isValid() );
		ensureReplyPrototype()->m_finishDelaySignal = signalConnection;
		return *this;
	}

	/*! \overload
	 *
	 * \tparam PointerToMemberFunction The type of the \p signal.
	 * \param sender The QObject which emits the signal to wait for with the `finished()` signal.
	 * \param signalPointer The signal to wait for as a function pointer.
	 * \param connectionType The type of the connection.
	 * \return A reference to this %MockReplyBuilder.
	 */
	template< typename PointerToMemberFunction >
	MockReplyBuilder& withFinishDelayUntil( QObject* sender,
	                                        PointerToMemberFunction signalPointer,
	                                        Qt::ConnectionType connectionType = Qt::AutoConnection )
	{
		const auto signalMetaMethod = QMetaMethod::fromSignal( signalPointer );
		Q_ASSERT_X( sender->metaObject()->method( signalMetaMethod.methodIndex() ) == signalMetaMethod,
		            Q_FUNC_INFO,
		            QStringLiteral( "Signal '%1' does not belong to class '%2' of sender object." )
		                .arg( QString::fromUtf8( signalMetaMethod.name() ),
		                      QString::fromUtf8( sender->metaObject()->className() ) )
		                .toLatin1()
		                .constData() );
		return withFinishDelayUntil( sender, signalMetaMethod, connectionType );
	}


	/*! Creates a reply using the configured properties.
	 * \return A new MockReply with properties as configured in this factory or a Q_NULLPTR if no properties have been
	 * configured.
	 */
	std::unique_ptr< MockReply > createReply() const
	{
		if ( m_replyPrototype )
			return m_replyPrototype->clone();
		else
			return nullptr;
	}

protected:
	/*! Creates a MockReply as prototype if necessary and returns it.
	 * \return A MockReply which acts as a prototype for the replies created by createReply().
	 * Modify the properties of the returned reply to change the configuration of this factory.
	 * The ownership of the returned reply stays with the MockReplyBuilder so do not delete it.
	 */
	MockReply* ensureReplyPrototype()
	{
		if ( ! m_replyPrototype )
		{
			m_replyPrototype.reset( new MockReply() );
		}
		return m_replyPrototype.get();
	}

private:
	void checkErrorAndStatusCodeConsistency() const
	{
		Q_ASSERT( m_replyPrototype );
		const auto statusCodeVariant = m_replyPrototype->attribute( QNetworkRequest::HttpStatusCodeAttribute );
		if ( ! statusCodeVariant.isNull() && m_userDefinedError )
		{
			const int statusCode = statusCodeVariant.toInt();
			const QNetworkReply::NetworkError expectedError = HttpStatus::statusCodeToNetworkError( statusCode );
			if ( expectedError != m_replyPrototype->error() )
			{
				qCWarning( log ) << "HTTP status code and QNetworkReply::error() do not match!"
				                 << "Status code is" << statusCode << "which corresponds to error" << expectedError
				                 << "but actual error is" << m_replyPrototype->error();
			}
		}
	}

	std::unique_ptr< MockReply > m_replyPrototype;
	bool m_userDefinedError = false;
};

/*! Configuration object for the Manager.
 *
 * The Rule combines predicates for matching requests with a MockReplyBuilder which generates MockReplies when the
 * predicates match.
 *
 * ### Usage ###
 * The Rule implements a chainable interface. This means that the methods return a reference to the Rule
 * itself to allow calling its methods one after the other in one statement.
 * Additionally, the Manager provides convenience methods to create Rule objects.
 * So the typical way to work with Rules is:
\code
using namespace MockNetworkAccess;
using namespace MockNetworkAccess::Predicates;
Manager< QNetworkAccessManager > mockNAM;

mockNAM.whenGet( QUrl( "http://example.com" ) )
       .has( HeaderMatching( QNetworkRequest::UserAgentHeader, QRegularExpression( ".*MyWebBrowser.*" ) ) )
       .reply().withBody( QJsonDocument::fromJson( "{\"response\": \"hello\"}" ) );
\endcode
 *
 * \note Rule objects cannot be copied but they can be cloned. See clone().
 *
 * ### Matching ###
 * To add predicates to a Rule, use the has() and hasNot() methods.
 * For a Rule to match a request, all its predicates must match. So the predicates have "and" semantics.
 * To achieve "or" semantics, configure multiple Rule in the Manager or implement a dynamic predicate (see
 * \ref page_dynamicMockNam_dynamicPredicates).
 * Since the first matching Rule in the Manager will be used to create a reply, this provides "or" semantics.
 * In addition to negating single Predicates (see hasNot() or Predicate::negate()), the matching of the whole Rule
 * object can be negated by calling negate().
 * \note
 * \parblock
 * The order of the Predicates in a Rule has an impact on the performance of the matching.
 * So, fast Predicates should be added before complex Predicates (for example, Predicates::Header before
 * Predicates::BodyMatching).
 * \endparblock
 *
 * ### Creating Replies ###
 * When a Rule matches a request, the Manager will request it to create a reply for the request.
 * The actual creation of the reply will be done by the Rule's MockReplyBuilder which can be accessed through the
 * reply() method.
 *
 * ### Extending Rule ###
 * Both the matching of requests and the generation of replies can be extended and customized.
 * To extend the matching, implement new Predicate classes.
 * To extend or customize the generation of replies, override the createReply() method. You can then use a
 * MockReplyBuilder to create a reply based on the request.
 * These extension possibilities allow implementing dynamic matching and dynamic replies. That is, depending on the
 * concrete values of the request, the matching behaves differently or the reply has different properties.
 * This also allows introducing state and effectively evolves the Rule into a simple fake server.\n
 * See \ref page_dynamicMockNam for further details.
 */
class Rule
{
	template< class Base >
	friend class Manager;

public:
	/*! Smart pointer to a Rule object. */
	using Ptr = QSharedPointer< Rule >;

	/*! Abstract base class for request matching.
	 * A Predicate defines a condition which a request must match.
	 * If all Predicates of a Rule match the request, the Rule is
	 * considered to match the request.
	 *
	 * To create custom Predicates, derive from this class and implement the private match() method.
	 */
	class Predicate
	{
	public:
		/*! Smart pointer to a Predicate. */
		using Ptr = QSharedPointer< Predicate >;

		/*! Default destructor
		 */
		virtual ~Predicate() = default;

		/*! Matches a request against this Predicate.
		 * \param request The request to test against this predicate.
		 * \return \c true if the Predicate matches the \p request.
		 */
		bool matches( const Request& request )
		{
			return match( request ) != m_negate;
		}

		/*! Negates the matching of this Predicate.
		 * \param negate If \c true, the result of matches() is negated before returned.
		 */
		void negate( bool negate = true )
		{
			m_negate = negate;
		}

	private:
		/*! Performs the actual matching.
		 * This method is called by matches() to do the actual matching.
		 * \param request The request to be tested to match this %Predicate.
		 * \return Must return \c true if the Predicate matches the \p request. Otherwise, \c false.
		 */
		virtual bool match( const Request& request ) = 0;

		bool m_negate = false;
	};

	/*! This enum defines the behaviors of a Rule regarding forwarding matching requests to the next network access
	 * manager.
	 */
	enum ForwardingBehavior
	{
		DontForward,                   /*!< The rule consumes matching requests and the Manager returns a MockReply
		                                * generated by the MockReplyBuilder of the rule (see reply()).
		                                * The request is **not** forwarded.\n
		                                * This is the default behavior.
		                                */
		ForwardAndReturnMockReply,     /*!< The rule forwards matching requests to the next network access manager but
		                                * the Manager still returns a MockReply generated by the MockReplyBuilder of the
		                                * rule (see reply()).
		                                * The reply returned by the next network access manager is discarded.
		                                * \note If the rule has no reply() configured, matching requests will not
		                                * be forwarded since the Rule is considered "invalid".
		                                */
		ForwardAndReturnDelegatedReply /*!< The rule forwards matching requests to the next network access manager and
		                                * the Manager returns the reply returned by the next network access manager.
		                                */
	};

	/*! Creates a Rule which matches every request but creates no replies.
	 *
	 * In regard to the Manager, such a Rule is invalid and is ignored by the Manager.
	 * To make it valid, configure the MockReplyBuilder returned by reply().
	 * \sa Manager
	 */
	Rule() = default;

	/*! Deleted copy constructor.
	 */
	Rule( const Rule& ) = delete;

	/*! Default move operator.
	 */
	Rule( Rule&& ) = default;

	/*! Default destructor.
	 */
	virtual ~Rule() = default;

	/*! Deleted assignment operator.
	 */
	Rule& operator=( const Rule& ) = delete;

	/*! Negates the matching of this rule.
	 * \param negate If \c true, the result of the matching is negated, meaning if _any_ of the predicates does _not_
	 * match, this Rule matches.
	 * If \c false, the negation is removed reverting to normal "and" semantics.
	 * \return A reference to this %Rule.
	 * \sa matches()
	 */
	Rule& negate( bool negate = true )
	{
		m_negate = negate;
		return *this;
	}

	/*! \return \c true if this rule negates the matching. \c false otherwise.
	 *
	 * \sa negate()
	 */
	bool isNegated() const
	{
		return m_negate;
	}

	/*! Adds a Predicate to the Rule.
	 * \tparam PredicateType The type of the \p predicate. \p PredicateType must be move-constructable (if
	 * \p predicate is an rvalue reference) or copy-constructable (if \p predicate is an lvalue reference) for
	 * this method to work.
	 * \param predicate The Predicate to be added to the Rule.
	 * Note that \p predicate will be copied/moved and the resulting Predicate is actually added to the Rule.
	 * \return A reference to this %Rule.
	 */
	template< class PredicateType >
	Rule& has( PredicateType&& predicate )
	{
		m_predicates.append( Predicate::Ptr(
		    new typename std::remove_const< typename std::remove_reference< PredicateType >::type >::type(
		        std::forward< PredicateType >( predicate ) ) ) );
		return *this;
	}


	/*! Adds a Predicate to the Rule.
	 * \param predicate Smart pointer to the Predicate to be added to the Rule.
	 * \return A reference to this %Rule.
	 */
	Rule& has( const Predicate::Ptr& predicate )
	{
		m_predicates.append( predicate );
		return *this;
	}

	/*! Negates a Predicate and adds it to the Rule.
	 * \tparam PredicateType The type of the \p predicate. \p PredicateType must be move-constructable (if
	 * \p predicate is an rvalue reference) or copy-constructable (if \p predicate is an lvalue reference) for
	 * this method to work.
	 * \param predicate The Predicate to be negated and added to the Rule.
	 * Note that \p predicate will be copied and the copy is negated and added.
	 * \return A reference to this %Rule.
	 */
	template< class PredicateType >
	Rule& hasNot( PredicateType&& predicate )
	{
		auto copy = QSharedPointer< typename std::remove_const< typename std::remove_reference< PredicateType >::type >::type >::create(
		    std::forward< PredicateType >( predicate ) );
		copy->negate();
		m_predicates.append( copy );
		return *this;
	}

	/*! Negates a Predicate and adds it to the Rule.
	 * \param predicate Smart pointer to the Predicate to be negated and added to the Rule.
	 * \return A reference to this %Rule.
	 * \sa Predicate::negate()
	 */
	Rule& hasNot( const Predicate::Ptr& predicate )
	{
		predicate->negate();
		m_predicates.append( predicate );
		return *this;
	}

	/*! Creates a \link Predicates::Generic Generic Predicate \endlink and adds it to this Rule.
	 *
	 * Example:
	 * \code
	 * Manager< QNetworkAccessManager > mnam;
	 * mnam.whenPost( QUrl( "http://example.com/json" ) )
	 *     .isMatching( [] ( const Request& request ) -> bool {
	 *         if ( request.body.isEmpty()
	 *           || request.qRequest.header( QNetworkRequest::ContentTypeHeader ).toString() != "application/json" )
	 *             return true;
	 *         QJsonDocument jsonDoc = QJsonDocument::fromJson( request.body );
	 *         return jsonDoc.isNull();
	 *     } )
	 *     .reply().withError( QNetworkReply::ProtocolInvalidOperationError, "Expected a JSON body" );
	 * \endcode
	 *
	 * \tparam Matcher The type of the callable object.
	 * \param matcher The callable object used to create the Generic predicate.
	 * \return A reference to this %Rule.
	 * \sa isNotMatching()
	 * \sa Predicates::Generic
	 * \sa Predicates::createGeneric()
	 */
	template< class Matcher >
	Rule& isMatching( const Matcher& matcher );

	/*! Creates a \link Predicates::Generic Generic Predicate \endlink, negates it and adds it to this Rule.
	 *
	 * See isMatching() for a usage example.
	 *
	 * \tparam Matcher The type of the callable object.
	 * \param matcher The callable object used to create the Generic predicate.
	 * \return A reference to this %Rule.
	 * \sa isMatching()
	 * \sa Predicates::Generic
	 * \sa Predicates::createGeneric()
	 */
	template< class Matcher >
	Rule& isNotMatching( const Matcher& matcher );

	/*! \return The predicates of this Rule.
	 */
	QVector< Predicate::Ptr > predicates() const
	{
		return m_predicates;
	}

	/*! Sets the predicates of this Rule.
	 * This removes all previous Predicates of this Rule.
	 * \param predicates The new Predicates for this Rule.
	 */
	void setPredicates( const QVector< Predicate::Ptr >& predicates )
	{
		m_predicates = predicates;
	}

	/*! \return The MockReplyBuilder used to create replies in case this Rule matches. Use the returned builder to
	 * configure the replies.
	 */
	MockReplyBuilder& reply()
	{
		return m_replyBuilder;
	}

	/*! Defines whether matching requests should be forwarded to the next network access manager.
	 * \param behavior How the Rule should behave in regard to forwarding requests.
	 * \param forwardingTargetManager The network access manager to which requests are forwarded.
	 * If this is null, the request is forwarded to the "forward target manager" of the Manager handling the request
	 * (see Manager::setForwardNam()).
	 * \n **Since** 0.4.0
	 * \return A reference to this %Rule.
	 * \sa ForwardingBehavior
	 * \sa \ref page_forwarding
	 */
	Rule& forward( ForwardingBehavior behavior = ForwardAndReturnDelegatedReply,
	               QNetworkAccessManager* forwardingTargetManager = Q_NULLPTR )
	{
		m_forwardingBehavior = behavior;
		m_forwardingTargetManager = forwardingTargetManager;
		return *this;
	}

	/*! \return Whether this rule forwards matching requests to the next network access manager and what
	 * is returned by the Manager if the request is forwarded.
	 *
	 * \sa ForwardingBehavior
	 */
	ForwardingBehavior forwardingBehavior() const
	{
		return m_forwardingBehavior;
	}

	/*! \return The network access manager to which matching requests are forwarded.
	 * \sa forward()
	 * \sa ForwardingBehavior
	 * \since 0.4.0
	 */
	QNetworkAccessManager* forwardingTargetManager() const
	{
		return m_forwardingTargetManager;
	}

	/*! Matches a request against the predicates of this Rule.
	 * \param request The request to be tested against the predicates.
	 * \return \c true if the \p request matches all predicates.
	 */
	bool matches( const Request& request ) const
	{
		bool returnValue = true;

		for ( auto&& predicate : m_predicates )
		{
			if ( ! predicate->matches( request ) )
			{
				returnValue = false;
				break;
			}
		}

		return returnValue != m_negate;
	}

	/*! Creates a MockReply using the MockReplyBuilder of this Rule.
	 *
	 * The base implementation simply calls MockReplyBuilder::createReply().
	 *
	 * \note When you reimplement this method, you can also return a null pointer. In that case, it is treated as if the
	 * Rule didn't match the request. This is useful if you create the replies dynamically and get into a
	 * situation where you cannot generate an appropriate reply.
	 *
	 * \param request The request to be answered.
	 * \return A new MockReply object created by the MockReplyBuilder (see reply()).
	 */
	virtual std::unique_ptr< MockReply > createReply( const Request& request )
	{
		Q_UNUSED( request )
		return m_replyBuilder.createReply();
	}

	/*! Creates a clone of this Rule.
	 *
	 * \return A Rule object with the same properties as this Rule except for the matchedRequests().
	 * Note that the predicates() are shallow copied meaning that this Rule and the clone will have pointers to
	 * the same Predicate objects. All other properties except for the matchedRequests() are copied.
	 */
	virtual std::unique_ptr< Rule > clone() const
	{
		auto cloned = std::unique_ptr< Rule >{ new Rule{} };
		cloned->m_predicates = m_predicates;
		cloned->m_replyBuilder = m_replyBuilder;
		cloned->m_negate = m_negate;
		cloned->m_forwardingBehavior = m_forwardingBehavior;
		cloned->m_forwardingTargetManager = m_forwardingTargetManager;
		return cloned;
	}

	/*! \return The requests that matched this Rule.
	 *
	 * \sa clearMatchedRequests()
	 */
	QVector< Request > matchedRequests() const
	{
		return m_matchedRequests;
	}

	/*! Deletes all recorded matched requests.
	 *
	 * \since 1.0.0
	 * \sa matchedRequests()
	 */
	void clearMatchedRequests()
	{
		m_matchedRequests.clear();
	}


private:
	QVector< Predicate::Ptr > m_predicates;
	MockReplyBuilder m_replyBuilder;
	bool m_negate = false;
	ForwardingBehavior m_forwardingBehavior = DontForward;
	QVector< Request > m_matchedRequests;
	QPointer< QNetworkAccessManager > m_forwardingTargetManager;
};


/*! Namespace for the matching predicates provided by the MockNetworkAccessManager library.
 * \sa Rule::Predicate
 */
namespace Predicates {
	/*! Matches any request.
	 * This is useful to handle unexpected requests.
	 */
	class Anything : public Rule::Predicate
	{
	public:
		using Rule::Predicate::Predicate;

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& ) override
		{
			return true;
		}
		//! \endcond
	};

	/*! Matches if a given callable object matches the request.
	 *
	 * Normally, this class does not need to be used directly since there are the
	 * convenience methods Rule::isMatching() and Rule::isNotMatching().
	 *
	 * If this class should still be used directly and the compiler does not support
	 * class template argument deduction, there is the convenience method createGeneric().
	 *
	 * \tparam Matcher A callable type which is used to match the request.
	 * The \p Matcher must accept a `const Request&` as parameter and return a `bool`.
	 * When the predicate is tested against a request, the \p Matcher is invoked
	 * and its return value defines whether this predicate matches.
	 *
	 * \sa createGeneric()
	 * \sa \ref page_dynamicMockNam_dynamicPredicates_examples_2
	 */
	template< class Matcher >
	class Generic : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching using a callable object.
		 * \param matcher The callable object which is invoked to match the request.
		 */
		explicit Generic( const Matcher& matcher )
		    : Predicate()
		    , m_matcher( matcher )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			return m_matcher( request );
		}
		//! \endcond

		Matcher m_matcher;
	};

	/*! Creates a Generic predicate.
	 * This factory method mainly exists to take advantage of template argument deduction when creating a Generic
	 * predicate.
	 * \tparam Matcher The type of the callable object. Must take a single \c const Request& parameter and
	 * return a \c bool.
	 * \param matcher The callable object. Must return \c true if the predicate matches the Request given as parameter.
	 * \return A smart pointer to a Generic predicate created with \p matcher.
	 * \sa Generic
	 */
	template< class Matcher >
	inline Rule::Predicate::Ptr createGeneric( const Matcher& matcher )
	{
		return Rule::Predicate::Ptr( new Generic< Matcher >( matcher ) );
	}

	/*! Matches if the HTTP verb equals a given verb.
	 */
	class Verb : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching the HTTP verb.
		 * \param operation The verb to match.
		 * \param customVerb If \p operation is QNetworkAccessManager::CustomOperation, \p customVerb defines the
		 * custom verb to match.
		 * In other cases, this parameter is ignored.
		 */
		explicit Verb( QNetworkAccessManager::Operation operation, const QByteArray& customVerb = QByteArray() )
		    : Predicate()
		    , m_operation( operation )
		{
			if ( m_operation == QNetworkAccessManager::CustomOperation )
				m_customVerb = customVerb;
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			if ( request.operation != m_operation )
				return false;
			if ( request.operation == QNetworkAccessManager::CustomOperation
			     && request.qRequest.attribute( QNetworkRequest::CustomVerbAttribute ).toByteArray() != m_customVerb )
				return false;
			return true;
		}
		//! \endcond

		QNetworkAccessManager::Operation m_operation;
		QByteArray m_customVerb;
	};

	/*! Matches if the request URL matches a regular expression.
	 * \note To match query parameters, it is typically easier to use the predicate QueryParameters.
	 */
	class UrlMatching : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching the request URL against a regular expression.
		 * \param urlRegEx The regular expression.
		 * \param format QUrl::FormattingOptions to be used to convert the QUrl to a QString when matching the regular
		 * expression.
		 * The default is QUrl::PrettyDecoded since it is also the default for QUrl::toString().
		 * Note that QUrl::FullyDecoded does *not* work since QUrl::toString() does not permit it.
		 */
		explicit UrlMatching( const QRegularExpression& urlRegEx,
		                      QUrl::FormattingOptions format = QUrl::FormattingOptions( QUrl::PrettyDecoded ) )
		    : Predicate()
		    , m_urlRegEx( urlRegEx )
		    , m_format( format )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			const QString url = request.qRequest.url().toString( m_format );
			return m_urlRegEx.match( url ).hasMatch();
		}
		//! \endcond

		QRegularExpression m_urlRegEx;
		QUrl::FormattingOptions m_format;
	};

	/*! Matches if the request URL equals a given URL.
	 * \note This predicate does an exact matching of the URL so it is stricter than the other URL predicates.
	 */
	class Url : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching if the request URL equals a given URL.
		 * \note Invalid QUrls are treated like empty QUrls for the comparison.
		 * In other words, the following QUrl objects are all considered equal: `QUrl()`, `QUrl("")`,
		 * `QUrl("http://..")`, `QUrl("http://!!")`
		 * \param url The URL which is compared against the request URL.
		 * \param defaultPort Allows defining a default port to be considered when the request or \p url does not
		 * specify a port explicitly.
		 * The default ports for HTTP (80), HTTPS (443) and FTP (21) are used when no \p defaultPort was
		 * specified (that is, when \p defaultPort is -1) and the \p url has a matching scheme.
		 */
		explicit Url( const QUrl& url, int defaultPort = -1 )
		    : Predicate()
		    , m_url( url )
		    , m_defaultPort( defaultPort )
		{
			detectDefaultPort();
		}

		/*! \overload
		 *
		 * \param url The URL compared against the request URL. If it is empty, it always matches.
		 * \param defaultPort Allows defining a default port to be considered when the request or \p url does not
		 * specify a port explicitly.
		 * The default ports for HTTP (80) and HTTPS (443) are used when no \p defaultPort was specified.
		 */
		explicit Url( const QString& url, int defaultPort = -1 )
		    : Predicate()
		    , m_url( url )
		    , m_defaultPort( defaultPort )
		{
			detectDefaultPort();
		}

	private:
		void detectDefaultPort()
		{
			if ( m_defaultPort == -1 )
				m_defaultPort = detail::defaultPortForScheme( m_url.scheme() );
		}

		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			const QUrl requestUrl = request.qRequest.url();
			return ( requestUrl == m_url )
			       || ( m_defaultPort > -1
			            /* QUrl::matches() could be used here instead of QUrl::adjusted() but it is buggy:
			             * https://bugreports.qt.io/browse/QTBUG-70774
			            && m_url.matches(requestUrl, QUrl::RemovePort)
			             */
			            && m_url.adjusted( QUrl::RemovePort ) == requestUrl.adjusted( QUrl::RemovePort )
			            && m_url.port( m_defaultPort ) == requestUrl.port( m_defaultPort ) );
		}
		//! \endcond

		QUrl m_url;
		int m_defaultPort;
	};

	/*! Matches if the request URL contains a given query parameter.
	 * Note that the URL can contain more query parameters. This predicate just checks that the given parameter exists
	 * with the given value.
	 *
	 * This predicate is especially useful in combination with the regular expression predicate UrlMatching()
	 * since query parameters typically don't have a defined order which makes it very hard to match them with regular
	 * expressions.
	 */
	class QueryParameter : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching a URL query parameter.
		 * \param key The name of the query parameter.
		 * \param value The value that the query parameter needs to have.
		 * \param format QUrl::ComponentFormattingOptions used to convert the query parameter value to a QString.
		 * The default is QUrl::PrettyDecoded since it is also the default for QUrlQuery::queryItemValue().
		 */
		explicit QueryParameter(
		    const QString& key,
		    const QString& value,
		    QUrl::ComponentFormattingOptions format = QUrl::ComponentFormattingOptions( QUrl::PrettyDecoded ) )
		    : Predicate()
		    , m_key( key )
		    , m_values( value )
		    , m_format( format )
		{
		}

		/*! Creates a predicate matching a URL query parameter with a list of values.
		 * \param key The name of the query parameter.
		 * \param values The values that the query parameter needs to have in the order they appear in the query.
		 * \param format QUrl::ComponentFormattingOptions used to convert the query parameter value to a QString.
		 * The default is QUrl::PrettyDecoded since it is also the default for QUrlQuery::queryItemValue().
		 * \since 0.4.0
		 */
		explicit QueryParameter(
		    const QString& key,
		    const QStringList& values,
		    QUrl::ComponentFormattingOptions format = QUrl::ComponentFormattingOptions( QUrl::PrettyDecoded ) )
		    : Predicate()
		    , m_key( key )
		    , m_values( values )
		    , m_format( format )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			const QUrlQuery query( request.qRequest.url() );
			return query.hasQueryItem( m_key ) && query.allQueryItemValues( m_key, m_format ) == m_values;
		}
		//! \endcond

		QString m_key;
		QStringList m_values;
		QUrl::ComponentFormattingOptions m_format;
	};

	/*! Matches if the request URL contains a given query parameter with a value matching a given regular expression.
	 * If the query parameter contains multiple values, **all** of its values must match the given regular expression.
	 *
	 * Note that the URL can contain more query parameters. This predicate just checks that the given parameter exists
	 * with a matching value.
	 *
	 * This predicate is especially useful in combination with the regular expression predicate UrlMatching()
	 * since query parameters typically don't have a defined order which makes it very hard to match them with regular
	 * expressions.
	 */
	class QueryParameterMatching : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching an URL query parameter value .
		 * \param key The name of the query parameter.
		 * \param regEx The regular expression matched against the query parameter value.
		 * \param format QUrl::ComponentFormattingOptions to be used to convert the query parameter value to a QString
		 * when matching the regular expression. The default is QUrl::PrettyDecoded since it is also the default for
		 * QUrlQuery::queryItemValue().
		 */
		explicit QueryParameterMatching(
		    const QString& key,
		    const QRegularExpression& regEx,
		    QUrl::ComponentFormattingOptions format = QUrl::ComponentFormattingOptions( QUrl::PrettyDecoded ) )
		    : Predicate()
		    , m_key( key )
		    , m_regEx( regEx )
		    , m_format( format )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			const QUrlQuery query( request.qRequest.url() );
			if ( ! query.hasQueryItem( m_key ) )
				return false;

			const auto values = query.allQueryItemValues( m_key );
			for ( auto&& value : values )
			{
				if ( ! m_regEx.match( value ).hasMatch() )
					return false;
			}
			return true;
		}
		//! \endcond

		QString m_key;
		QRegularExpression m_regEx;
		QUrl::ComponentFormattingOptions m_format;
	};

	/*! Matches if the request URL contains given query parameters.
	 * Note that the URL can contain more query parameters. This predicate just checks that the given parameters exist
	 * with the given values.
	 *
	 * This predicate is especially useful in combination with the regular expression predicate UrlMatching()
	 * since query parameters typically don't have a defined order which makes it very hard to match them with regular
	 * expressions.
	 */
	class QueryParameters : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching URL query parameters.
		 * \param parameters A QHash of query parameters that need to be present in the URL with defined values.
		 * The keys of the hash are the expected parameter names and the corresponding values of the hash are the
		 * expected parameter values.
		 * \param format QUrl::ComponentFormattingOptions used to convert the query parameter value to a QString.
		 * The default is QUrl::PrettyDecoded since it is also the default for QUrlQuery::queryItemValue().
		 */
		explicit QueryParameters(
		    const QueryParameterHash& parameters,
		    QUrl::ComponentFormattingOptions format = QUrl::ComponentFormattingOptions( QUrl::PrettyDecoded ) )
		    : Predicate()
		    , m_format( format )
		{
			auto iter = parameters.cbegin();
			const auto end = parameters.cend();
			for ( ; iter != end; ++iter )
			{
				m_queryParameters.insert( iter.key(), QStringList() << iter.value() );
			}
		}

		/*! Creates a predicate matching URL query parameters.
		 * \param parameters A QHash of query parameters that need to be present in the URL with defined values.
		 * The keys of the hash are the expected parameter names and the corresponding values of the hash are the
		 * expected parameter values in the order they appear in the query.
		 * \param format QUrl::ComponentFormattingOptions used to convert the query parameter value to a QString.
		 * The default is QUrl::PrettyDecoded since it is also the default for QUrlQuery::queryItemValue().
		 * \since 0.4.0
		 */
		explicit QueryParameters(
		    const MultiValueQueryParameterHash& parameters,
		    QUrl::ComponentFormattingOptions format = QUrl::ComponentFormattingOptions( QUrl::PrettyDecoded ) )
		    : Predicate()
		    , m_queryParameters( parameters )
		    , m_format( format )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			const QUrlQuery query( request.qRequest.url() );
			auto iter = m_queryParameters.cbegin();
			const auto end = m_queryParameters.cend();
			for ( ; iter != end; ++iter )
			{
				if ( ! query.hasQueryItem( iter.key() )
				     || query.allQueryItemValues( iter.key(), m_format ) != iter.value() )
				{
					return false;
				}
			}
			return true;
		}
		//! \endcond

		MultiValueQueryParameterHash m_queryParameters;
		QUrl::ComponentFormattingOptions m_format;
	};

	/*! Matches if *all* URL query parameters match one of the given regular expression pairs.
	 *
	 * This predicates checks all URL query parameters against the given regular expression pairs in the order
	 * they are given. If the first regular expression of a pair matches the name of the query parameter, then the
	 * second regular expression must match the value of the parameter. If the value does not match or if the parameter
	 * name does not match any of the first regular expressions of the pairs, then the predicate does not match.
	 * If all query parameter names match one of the first regular expressions and the parameter values match the
	 * corresponding second regular expression, then this predicate matches.
	 *
	 * Note that for parameters with multiple values, all values of the parameter need to match the second regular
	 * expression.
	 *
	 * This predicate can be used to ensure that there are not unexpected query parameters.
	 */
	class QueryParameterTemplates : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching all query parameters against regular expression pairs.
		 *
		 * \param templates QVector of QRegularExpression pairs. The first regular expressions are matched against the
		 * query parameter names and the second regular expressions are matched against the query parameter values.
		 * \param format QUrl::ComponentFormattingOptions used to convert the query parameter value to a QString.
		 * The default is QUrl::PrettyDecoded since it is also the default for QUrlQuery::queryItemValue().
		 */
		explicit QueryParameterTemplates(
		    const RegExPairVector& templates,
		    QUrl::ComponentFormattingOptions format = QUrl::ComponentFormattingOptions( QUrl::PrettyDecoded ) )
		    : Predicate()
		    , m_templates( templates )
		    , m_format( format )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			const QUrlQuery query( request.qRequest.url() );
			const auto queryParams = query.queryItems( m_format );

			for ( auto&& queryParamPair : queryParams )
			{
				bool matched = false;

				const auto& templates = m_templates;
				for ( auto&& templatePair : templates )
				{
					if ( templatePair.first.match( queryParamPair.first ).hasMatch() )
					{
						matched = templatePair.second.match( queryParamPair.second ).hasMatch();
						break;
					}
				}

				if ( ! matched )
					return false;
			}

			return true;
		}
		//! \endcond

		RegExPairVector m_templates;
		QUrl::ComponentFormattingOptions m_format;
	};

	/*! Matches if the request body matches a regular expression.
	 *
	 * To match against the regular expression, the body needs to be converted to a QString.
	 * If a \p codec is provided in the constructor, it is used to convert the body.
	 * Else, the predicate tries to determine the codec from the [QNetworkRequest::ContentTypeHeader][]:
	 * - If the content type header contains codec information using the `"charset:<CODEC>"` format, this codec is used,
	 * if supported.
	 *   - If the codec is not supported, a warning is printed and the predicate falls back to Latin-1.
	 * - If the content type header does not contain codec information, the MIME type is investigated.
	 *   - If the MIME type is known and
	 * inherits from `text/plain`, the predicate uses QTextCodec::codecForUtfText() to detect the codec and falls back
	 * to UTF-8 if the codec cannot be detected.
	 * - In all other cases, including the case that there is no content type header at all and the case that the
	 * content is binary, the predicate uses QTextCodec::codecForUtfText() to detect the codec and falls back to
	 * Latin-1 if the codec cannot be detected.
	 * \note
	 * \parblock
	 * When trying to match without using the correct codec, (for example, when matching binary content), the regular
	 * expression patterns must be aware of the codec mismatch. In such cases, the best approach is to use the
	 * numerical value of the encoded character.
	 * For example, matching the character "ç" (LATIN SMALL LETTER C WITH CEDILLA) encoded in UTF-8 when the predicate
	 * uses Latin-1 encoding would require the pattern \c "Ã§" assuming the pattern itself is encoded using UTF-8.
	 * Since this can lead to mistakes easily, one should rather use the pattern \c "\\xC3\\x83".
	 * \endparblock
	 *
	 * \sa QMimeDatabase
	 * [QNetworkRequest::ContentTypeHeader]: http://doc.qt.io/qt-5/qnetworkrequest.html#KnownHeaders-enum
	 */
	class BodyMatching : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching the request body using a regular expression.
		 * \param bodyRegEx The regular expression to match against the request body.
		 * \param decoder The decoder to be used to convert the body into a QString. If null, the predicate
		 * tries to determine the codec based on the [QNetworkRequest::ContentTypeHeader] or based on the
		 * request body. The BodyMatching instance does **not** take ownership of the \p decoder.
		 * [QNetworkRequest::ContentTypeHeader]: http://doc.qt.io/qt-5/qnetworkrequest.html#KnownHeaders-enum
		 */
		explicit BodyMatching( const QRegularExpression& bodyRegEx, StringDecoder decoder = StringDecoder() )
		    : Predicate()
		    , m_bodyRegEx( bodyRegEx )
		    , m_decoder( decoder )
		    , m_charsetFieldRegEx( QStringLiteral( "charset:(.*)" ) )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			if ( ! m_decoder.isValid() )
				determineDecoder( request );

			const QString decodedBody = m_decoder.decode( request.body );

			return m_bodyRegEx.match( decodedBody ).hasMatch();
		}
		//! \endcond

		void determineDecoder( const Request& request ) const
		{
			determineDecoderFromContentType( request );

			if ( ! m_decoder.isValid() )
				determineDecoderFromBody( request.body );
		}

		void determineDecoderFromContentType( const Request& request ) const
		{
			const auto contentTypeHeader = request.qRequest.header( QNetworkRequest::ContentTypeHeader ).toString();
			if ( contentTypeHeader.isEmpty() )
				return;

			auto contentTypeFields = contentTypeHeader.split( QChar::fromLatin1( ';' ) );
			const auto charsetFieldIndex = contentTypeFields.indexOf( m_charsetFieldRegEx );
			if ( charsetFieldIndex >= 0 )
			{
				const auto& charsetField = contentTypeFields.at( charsetFieldIndex );
				const auto charset = HttpUtils::trimmed( m_charsetFieldRegEx.match( charsetField ).captured( 1 ) );
				determineDecoderFromCharset( charset );
			}
			else
			{
				const auto mimeType = QMimeDatabase().mimeTypeForName( contentTypeFields.first() );
				if ( mimeType.inherits( QStringLiteral( "text/plain" ) ) )
					determineDecoderFromBody( request.body, QStringLiteral( "utf-8" ) );
			}
		}

		void determineDecoderFromCharset( const QString& charset ) const
		{
			m_decoder.setCodec( charset );
			if ( ! m_decoder.isValid() )
			{
				qCWarning( log ) << "Unsupported charset:" << charset;
				useFallbackDecoder();
			}
		}

		void determineDecoderFromBody( const QByteArray& body,
		                               const QString& fallbackCodec = QStringLiteral( "Latin-1" ) ) const
		{
			m_decoder.setCodecFromData( body, fallbackCodec );
			Q_ASSERT( m_decoder.isValid() );
		}

		void useFallbackDecoder() const
		{
			m_decoder.setCodec( QStringLiteral( "Latin-1" ) );
			Q_ASSERT( m_decoder.isValid() );
		}

		QRegularExpression m_bodyRegEx;
		mutable StringDecoder m_decoder;
		QRegularExpression m_charsetFieldRegEx;
	};

	/*! Match if the request body contains a given snippet.
	 */
	class BodyContaining : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching a snippet in the request body.
		 * \param bodySnippet The byte sequence that needs to exist in the request body.
		 */
		explicit BodyContaining( const QByteArray& bodySnippet )
		    : Predicate()
		    , m_bodySnippet( bodySnippet )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			return request.body.contains( m_bodySnippet );
		}
		//! \endcond

		QByteArray m_bodySnippet;
	};

	/*! Matches if the request body equals a given body.
	 * \note This predicate does an exact matching so it is stricter than the
	 * other body predicates.
	 */
	class Body : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching the request body.
		 * \param body The body to be compared to the request body.
		 */
		explicit Body( const QByteArray& body )
		    : Predicate()
		    , m_body( body )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			return request.body == m_body;
		}
		//! \endcond

		QByteArray m_body;
	};

	/*! Matches if the request contains given headers.
	 * Note that the request can contain more headers. This predicate just checks that the given headers exist with the
	 * given values.
	 * \note For this predicate to work correctly, the type of the header field must be registered with
	 * qRegisterMetaType() and QMetaType::registerComparators() or QMetaType::registerEqualsComparator().
	 * \sa QNetworkRequest::header()
	 */
	class Headers : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching a set of request headers.
		 * \param headers QHash of headers that need to be present in the request
		 * with defined values. The keys of the hash are the names of the expected
		 * headers and the corresponding values of the hash are the expected values
		 * of the headers.
		 */
		explicit Headers( const HeaderHash& headers )
		    : Predicate()
		    , m_headers( headers )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			auto iter = m_headers.cbegin();
			const auto end = m_headers.cend();
			for ( ; iter != end; ++iter )
			{
				if ( request.qRequest.header( iter.key() ) != iter.value() )
					return false;
			}
			return true;
		}
		//! \endcond

		HeaderHash m_headers;
	};

	/*! Match if the request contains a given header.
	 * Note that the request can contain more headers. This predicate just checks that the given header exists with the
	 * given value.
	 * \note For this predicate to work correctly, the type of the header field must be registered with
	 * qRegisterMetaType() and QMetaType::registerComparators() or QMetaType::registerEqualsComparator().
	 * \sa QNetworkRequest::header()
	 */
	class Header : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching a request header.
		 * \param header The header that needs to be present in the request.
		 * \param value The value that the \p header needs to have.
		 */
		explicit Header( QNetworkRequest::KnownHeaders header, const QVariant& value )
		    : Predicate()
		    , m_header( header )
		    , m_value( value )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			const auto headerValue = request.qRequest.header( m_header );
			return headerValue == m_value;
		}
		//! \endcond

		QNetworkRequest::KnownHeaders m_header;
		QVariant m_value;
	};

	/*! Matches if a header value matches a regular expression.
	 * \note
	 * \parblock
	 * - The \p header's value is converted to a string using QVariant::toString() to match it against the regular
	 *   expression.
	 * - This predicate does not distinguish between the case that the header has not been set and the case that the
	 *   header has been set to an empty value. So both cases match if the \p regEx matches empty strings.
	 * \endparblock
	 * \sa QNetworkRequest::header()
	 */
	class HeaderMatching : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching a header value using a regular expression.
		 * \param header The header whose value needs to match.
		 * \param regEx The regular expression matched against the \p header's value.
		 */
		explicit HeaderMatching( QNetworkRequest::KnownHeaders header, const QRegularExpression& regEx )
		    : Predicate()
		    , m_header( header )
		    , m_regEx( regEx )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			const auto headerValue = request.qRequest.header( m_header );
			return m_regEx.match( headerValue.toString() ).hasMatch();
		}
		//! \endcond

		QNetworkRequest::KnownHeaders m_header;
		QRegularExpression m_regEx;
	};

	/*! Matches if the request contains given raw headers.
	 * Note that the request can contain more headers. This predicate just checks that the given headers exist with the
	 * given values.
	 * \sa QNetworkRequest::rawHeader()
	 */
	class RawHeaders : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching a set of raw headers.
		 * \param rawHeaders QHash of raw headers that need to be present in the request with defined values.
		 * The keys of the hash are the names of the expected headers and
		 * the values of the hash are the corresponding expected values of the headers.
		 */
		explicit RawHeaders( const RawHeaderHash& rawHeaders )
		    : Predicate()
		    , m_rawHeaders( rawHeaders )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			auto iter = m_rawHeaders.cbegin();
			const auto end = m_rawHeaders.cend();
			for ( ; iter != end; ++iter )
			{
				if ( request.qRequest.rawHeader( iter.key() ) != iter.value() )
					return false;
			}
			return true;
		}
		//! \endcond

		RawHeaderHash m_rawHeaders;
	};

	/*! Matches if the request contains a given raw header.
	 * Note that the request can contain more headers. This predicate just checks that the given header exists with the
	 * given value.
	 * \sa QNetworkRequest::rawHeader()
	 */
	class RawHeader : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching a raw request header.
		 * \param header The raw header that needs to be present in the request.
		 * \param value The value that the \p header needs to have.
		 */
		explicit RawHeader( const QByteArray& header, const QByteArray& value )
		    : Predicate()
		    , m_header( header )
		    , m_value( value )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			return request.qRequest.rawHeader( m_header ) == m_value;
		}
		//! \endcond

		QByteArray m_header;
		QByteArray m_value;
	};

	/*! Matches if a raw header value matches a regular expression.
	 * \note
	 * \parblock
	 * - The \p header's value is converted to a string using QString::fromUtf8() to match it against the \p regEx.
	 * - This predicate does not distinguish between the case that the header has not been set and the case that the
	 *   header has been set to an empty value. So both cases match if the \p regEx matches empty strings.
	 * \endparblock
	 * \sa QNetworkRequest::rawHeader()
	 */
	class RawHeaderMatching : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching the value of a raw header using a regular expression.
		 * \param header The raw header whose value needs to match.
		 * \param regEx The regular expression matched against the \p header's value.
		 */
		explicit RawHeaderMatching( const QByteArray& header, const QRegularExpression& regEx )
		    : Predicate()
		    , m_header( header )
		    , m_regEx( regEx )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			const auto headerValue = QString::fromUtf8( request.qRequest.rawHeader( m_header ) );
			return m_regEx.match( headerValue ).hasMatch();
		}
		//! \endcond

		QByteArray m_header;
		QRegularExpression m_regEx;
	};


	/*! Matches if *all* request headers match one of the given regular expression pairs.
	 *
	 * This predicates checks all defined request headers against the given regular expression pairs in the order
	 * they are given. If the first regular expression of a pair matches the name of the header, then the
	 * second regular expression must match the value of the header. If the value does not match or if the header
	 * name does not match any of the first regular expressions of the pairs, then the predicate does not match.
	 * If all header names match one of the first regular expressions and the header values match the
	 * corresponding second regular expression, then this predicate matches.
	 *
	 * This predicate can be used to ensure that there are no unexpected headers.
	 *
	 * \note \parblock
	 * - This predicate also checks the headers defined using QNetworkRequest::setHeader().
	 * - Be aware that the Manager might add QNetworkCookies to the [QNetworkRequest::CookieHeader] in case
	 * [QNetworkRequest::CookieLoadControlAttribute] is set to [QNetworkRequest::Automatic].
	 * \endparblock
	 *
	 * ## Example ##
	 * \code
	 * RegExPairVector headerTemplates;
	 * headerTemplates.append( qMakePair( QRegularExpression( "^Accept.*" ), QRegularExpression( ".*" ) ) );
	 * headerTemplates.append( qMakePair( QRegularExpression( "^Host" ), QRegularExpression( ".*" ) ) );
	 * headerTemplates.append( qMakePair( QRegularExpression( "^User-Agent$" ), QRegularExpression( ".*" ) ) );
	 *
	 * mockNam.whenGet( QUrl( "http://example.com" ) )
	 *        .has( RawHeaderTemplates( headerTemplates ) )
	 *        .reply().withStatus( HttpStatus::OK );
	 * mockNam.whenGet( QUrl( "http://example.com" ) )
	 *        .reply().withError( QNetworkReply::UnknownContentError, "Unexpected header" );
	 * \endcode
	 *
	 * [QNetworkRequest::CookieHeader]: http://doc.qt.io/qt-5/qnetworkrequest.html#KnownHeaders-enum
	 * [QNetworkRequest::CookieLoadControlAttribute]: http://doc.qt.io/qt-5/qnetworkrequest.html#Attribute-enum
	 * [QNetworkRequest::Automatic]: http://doc.qt.io/qt-5/qnetworkrequest.html#LoadControl-enum
	 */
	class RawHeaderTemplates : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching all headers against regular expression pairs.
		 *
		 * \param templates QVector of QRegularExpression pairs. The first regular expressions are matched against the
		 * header names and the second regular expressions are matched against the header values.
		 */
		explicit RawHeaderTemplates( const RegExPairVector& templates )
		    : Predicate()
		    , m_templates( templates )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			const auto headerList = request.qRequest.rawHeaderList();
			for ( auto&& header : headerList )
			{
				bool matched = false;

				const auto& templates = m_templates;
				for ( auto&& templatePair : templates )
				{
					if ( templatePair.first.match( QString::fromUtf8( header ) ).hasMatch() )
					{
						const QByteArray headerValue = request.qRequest.rawHeader( header );

						matched = templatePair.second.match( QString::fromUtf8( headerValue ) ).hasMatch();
						break;
					}
				}

				if ( ! matched )
					return false;
			}

			return true;
		}
		//! \endcond

		RegExPairVector m_templates;
	};

	/*! Match if the request has a given attribute.
	 * Note that the request can have more attributes. This predicate just checks that the given attribute exists with
	 * the given value.
	 * \note
	 * \parblock
	 * - This predicate cannot match the default values of the attributes since QNetworkRequest::attribute()
	 * does not return the default values. As a workaround, use the \p matchInvalid flag: when you want to match the
	 * default value, set \p value to the default value and set \p matchInvalid to \c true. Then the predicate will
	 * match either when the attribute has been set to the default value explicitly or when the attribute has not been
	 * set at all and therefore falls back to the default value.
	 * - Since the attributes are an internal feature of %Qt and are never sent to a server, using this predicate means
	 * mocking the behavior of the QNetworkAccessManager instead of the server.
	 * \endparblock
	 * \sa QNetworkRequest::attribute()
	 */
	class Attribute : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching a request attribute.
		 * \param attribute The request attribute whose values is matched by this predicate.
		 * \param value The value that the \p attribute needs to have.
		 * \param matchInvalid If \c true, this predicate will match if the attribute has not been specified
		 * on the request. So the predicate matches if either the attribute has been set to the given \p value
		 * or not set at all. If \c false, this predicate will only match if the attribute has been set
		 * to the specified \p value explicitly.
		 */
		explicit Attribute( QNetworkRequest::Attribute attribute, const QVariant& value, bool matchInvalid = false )
		    : Predicate()
		    , m_attribute( attribute )
		    , m_value( value )
		    , m_matchInvalid( matchInvalid )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			const auto attribute = request.qRequest.attribute( m_attribute );
			return ( m_matchInvalid && ! attribute.isValid() ) || attribute == m_value;
		}
		//! \endcond

		QNetworkRequest::Attribute m_attribute;
		QVariant m_value;
		bool m_matchInvalid;
	};

	/*! Matches if a attribute value matches a regular expression.
	 * \note
	 * \parblock
	 * - The \p attributes's value is converted to a string using QVariant::toString() to match it against the regular
	 *   expression.
	 * - This predicate does not distinguish between the case that the attribute has not been set and the case that the
	 *   attribute has been set to an empty value. So both cases match if the \p regEx matches empty strings.
	 * - Since the attributes are an internal feature of %Qt and are never sent to a server, using this predicate means
	 *   mocking the behavior of the QNetworkAccessManager instead of the server.
	 * \endparblock
	 * \sa QNetworkRequest::attribute()
	 */
	class AttributeMatching : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching an attribute value using a regular expression.
		 * \param attribute The attribute whose value needs to match.
		 * \param regEx The regular expression matched against the \p attribute's value.
		 */
		explicit AttributeMatching( QNetworkRequest::Attribute attribute, const QRegularExpression& regEx )
		    : Predicate()
		    , m_attribute( attribute )
		    , m_regEx( regEx )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			const auto attributeValue = request.qRequest.attribute( m_attribute );
			return m_regEx.match( attributeValue.toString() ).hasMatch();
		}
		//! \endcond

		QNetworkRequest::Attribute m_attribute;
		QRegularExpression m_regEx;
	};

	/*! Matches if the request contains a specified Authorization header.
	 *
	 * In case an unsupported authentication method is required, you might use RawHeaderMatching to "manually" match
	 * authorized requests.
	 *
	 * For example to check for a bearer authorization:
	 * \code
	 * using namespace MockNetworkAccess;
	 *
	 * Rule authorizedRequestsRule;
	 * authorizedRequestsRule.has( Predicates::RawHeaderMatching( HttpUtils::authorizationHeader(), QRegularExpression(
	 * "Bearer .*" ) ) );
	 * \endcode
	 * \sa RawHeaderMatching
	 */
	class Authorization : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching an authorization using the HTTP Basic authentication scheme with given username
		 * and password.
		 * \param username The username that must be given.
		 * \param password The password that must be given.
		 */
		explicit Authorization( const QString& username, const QString& password )
		    : Predicate()
		{
			QAuthenticator authenticator;
			authenticator.setUser( username );
			authenticator.setPassword( password );
			m_authenticators.append( authenticator );
			m_authChallenge.reset( new HttpUtils::Authentication::Basic( QStringLiteral( "dummy" ) ) );
		}

		/*! Creates a predicate matching an authorization using the HTTP Basic authentication scheme with
		 * a selection of username and password combinations.
		 * \param credentials QHash of  username and password combinations. The authorization in the request must match
		 * one of these \p credentials.
		 */
		explicit Authorization( const QHash< QString, QString >& credentials )
		    : Predicate()
		{
			auto iter = credentials.cbegin();
			const auto end = credentials.cend();
			for ( ; iter != end; ++iter )
			{
				QAuthenticator authenticator;
				authenticator.setUser( iter.key() );
				authenticator.setPassword( iter.value() );
				m_authenticators.append( authenticator );
			}
			m_authChallenge.reset( new HttpUtils::Authentication::Basic( QStringLiteral( "dummy" ) ) );
		}

		/*! Creates a predicate matching an  authorization which matches a given authentication challenge with
		 * credentials defined by a given QAuthenticator.
		 * \param authChallenge The authentication challenge which the authorization in the request must match.
		 * \param authenticators Allowed username and password combinations. The authorization in the request must
		 * match one of these combinations.
		 */
		explicit Authorization( const HttpUtils::Authentication::Challenge::Ptr& authChallenge,
		                        const QVector< QAuthenticator >& authenticators )
		    : Predicate()
		    , m_authChallenge( authChallenge )
		    , m_authenticators( authenticators )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			const auto& authenticators = m_authenticators;
			for ( auto&& authenticator : authenticators )
			{
				if ( m_authChallenge->verifyAuthorization( request.qRequest, authenticator ) )
					return true;
			}

			return false;
		}
		//! \endcond


		HttpUtils::Authentication::Challenge::Ptr m_authChallenge;
		QVector< QAuthenticator > m_authenticators;
	};

	/*! Matches if a request contains a cookie with a given value.
	 * Note that the request can contain more cookies. This predicate just checks that the given cookie exists with the
	 * given value.
	 *
	 * \note
	 * \parblock
	 * - If there is no cookie with the given name, this predicate does not match.
	 * - In case there are multiple cookies with the given name, the first one is used and the other ones are ignored.
	 * \endparblock
	 *
	 * \sa [QNetworkRequest::CookieHeader]
	 * [QNetworkRequest::CookieHeader]: http://doc.qt.io/qt-5/qnetworkrequest.html#KnownHeaders-enum
	 */
	class Cookie : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching a cookie value.
		 * \param cookie The cookie which should exist. Only the QNetworkCookie::name() and QNetworkCookie::value()
		 * are used to match. Other properties of the cookie (like QNetworkCookie::domain() or
		 * QNetworkCookie::expiryDate()) are ignored.
		 */
		explicit Cookie( const QNetworkCookie& cookie )
		    : Predicate()
		    , m_cookie( cookie )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			const auto requestCookies = request.qRequest.header( QNetworkRequest::CookieHeader )
			                                .value< QList< QNetworkCookie > >();

			for ( auto& requestCookie : requestCookies )
			{
				/* We use the first matching cookie and ignore possible other cookies with the same name.
				 * RFC 6265 does not define a "correct" way to handle this but this seems to be the common practice.
				 * See https://stackoverflow.com/a/24214538/490560
				 */
				if ( requestCookie.name() == m_cookie.name() )
					return ( requestCookie.value() == m_cookie.value() );
			}

			return false;
		}
		//! \endcond

		QNetworkCookie m_cookie;
	};

	/*! Matches if a request contains a cookie with a value matching a regular expression.
	 * \note
	 * \parblock
	 * - The cookies's value is converted to a string using QString::fromUtf8() to match it against the \p regEx.
	 * - If there is no cookie with the given name, this predicate does not match, no matter what \p regEx is.
	 * - If the cookie's value is empty, it is matched against the \p regEx.
	 * - In case there are multiple cookies with the given name, the first one is used and the other ones are ignored.
	 * \endparblock
	 * \sa QNetworkRequest::rawHeader()
	 */
	class CookieMatching : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching the value of a cookie using a regular expression.
		 * \param cookieName The name of the cookie whose value needs to match.
		 * \param regEx The regular expression matched against the \p header's value.
		 */
		explicit CookieMatching( const QByteArray& cookieName, const QRegularExpression& regEx )
		    : Predicate()
		    , m_cookieName( cookieName )
		    , m_regEx( regEx )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			const auto cookies = request.qRequest.header( QNetworkRequest::CookieHeader ).value< QList< QNetworkCookie > >();
			for ( auto&& cookie : cookies )
			{
				const QByteArray cookieName = cookie.name();
				/* We use the first matching cookie and ignore possible other cookies with the same name.
				 * RFC 6265 does not define a "correct" way to handle this but this seems to be the common practice.
				 * See https://stackoverflow.com/a/24214538/490560
				 */
				if ( m_cookieName == cookieName )
				{
					const auto cookieValue = QString::fromUtf8( cookie.value() );
					return m_regEx.match( cookieValue ).hasMatch();
				}
			}
			return false;
		}
		//! \endcond

		QByteArray m_cookieName;
		QRegularExpression m_regEx;
	};


	/*! Matches if a request contains a JSON body equal to a given JSON document.
	 *
	 * If the request body is not a valid JSON document, then this predicate does not
	 * match even if the given JSON document is invalid as well.
	 *
	 * \note This predicate does an exact matching so it is stricter than the
	 * other body predicates.
	 *
	 * \since 0.5.0
	 * \sa JsonBodyContaining
	 */
	class JsonBody : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching a JSON body.
		 * \param body The body to be compared to the request body.
		 */
		explicit JsonBody( const QJsonDocument& body )
		    : Predicate()
		    , m_body( body )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			QJsonParseError error;
			const auto parsedDoc = QJsonDocument::fromJson( request.body, &error );
			if ( error.error != QJsonParseError::NoError )
				return false;

			return parsedDoc == m_body;
		}
		//! \endcond

		QJsonDocument m_body;
	};

	/*! Matches if a request contains a JSON body which contains the properties or entries of a given JSON document.
	 *
	 * This predicate does a recursive comparison of JSON object properties and array entries.
	 * So the predicate matches if the body of a request is a JSON document which contains at least the properties
	 * or entries of the JSON document given to the constructor.
	 *
	 * For example: Given the following JSON document as the request body:
	 * \code{.json}
	 * {
	 *     "prop1": "value 1",
	 *     "prop2": true,
	 *     "nested": {
	 *         "sub prop1": "value 2",
	 *         "sub prop2": 17,
	 *         "array prop": [
	 *              "value 3",
	 *              "value 4",
	 *              "value 5"
	 *         ]
	 *     }
	 * }
	 * \endcode
	 *
	 * Then this predicate would match when constructed with the following JSON documents:
	 * \code{.json}
	 * {
	 *     "prop1": "value 1",
	 * }
	 * \endcode
	 * \code{.json}
	 * {
	 *     "nested": {
	 *         "sub prop2": 17
	 *     }
	 * }
	 * \endcode
	 * \code{.json}
	 * {
	 *     "nested": {
	 *         "array prop": [
	 *              "value 4"
	 *         ]
	 *     }
	 * }
	 * \endcode
	 *
	 * However, it would fail when given the following JSON documents:
	 * \code{.json}
	 * [
	 *     "prop1"
	 * ]
	 * \endcode
	 * \code{.json}
	 * {
	 *     "prop2": false,
	 * }
	 * \endcode
	 * \code{.json}
	 * {
	 *     "nested": {
	 *         "array prop": [
	 *              "another value"
	 *         ]
	 *     }
	 * }
	 * \endcode
	 *
	 * \since 0.5.0
	 * \sa JsonBody
	 */
	class JsonBodyContaining : public Rule::Predicate
	{
	public:
		/*! Creates a predicate matching parts of a JSON body.
		 *
		 * \param bodyPart The properties or entries to be expected in the request body.
		 * \param ensureArrayOrder If \c true, array entries must appear in the same (relative) order
		 * in the request body as in \p bodyPart. If \c false, the order of the array entries does not matter,
		 * only the existence of the entries is verified. Note that even if this is \c true, there can still
		 * be other entries in the arrays of the request body.
		 */
		explicit JsonBodyContaining( const QJsonDocument& bodyPart, bool ensureArrayOrder = false )
		    : Predicate()
		    , m_bodyPart( bodyPart )
		    , m_ensureArrayOrder( ensureArrayOrder )
		{
		}

	private:
		//! \cond PRIVATE_IMPLEMENTATION
		bool match( const Request& request ) override
		{
			QJsonParseError error;
			const auto parsedDoc = QJsonDocument::fromJson( request.body, &error );
			if ( error.error != QJsonParseError::NoError )
				return false;

			if ( m_bodyPart.isArray() )
			{
				if ( ! parsedDoc.isArray() )
					return false;
				return matchArrays( parsedDoc.array(), m_bodyPart.array() );
			}
			if ( m_bodyPart.isObject() )
			{
				if ( ! parsedDoc.isObject() )
					return false;
				return matchObjects( parsedDoc.object(), m_bodyPart.object() );
			}

			// LCOV_EXCL_START
			Q_UNREACHABLE();
			return false;
			// LCOV_EXCL_STOP
		}
		//! \endcond

		bool matchValues( const QJsonValue& value, const QJsonValue& expectedValue )
		{
			if ( isSimpleValue( value ) )
				return value == expectedValue;

			if ( value.isArray() )
			{
				if ( ! expectedValue.isArray() )
					return false;

				return matchArrays( value.toArray(), expectedValue.toArray() ); // RECURSION !!!
			}

			if ( value.isObject() )
			{
				if ( ! expectedValue.isObject() )
					return false;

				return matchObjects( value.toObject(), expectedValue.toObject() ); // RECURSION !!!
			}

			// LCOV_EXCL_START
			Q_UNREACHABLE();
			return false;
			// LCOV_EXCL_STOP
		}

		static bool isSimpleValue( const QJsonValue& value )
		{
			return value.isString() || value.isBool() || value.isDouble() || isNullish( value );
		}

		static bool isNullish( const QJsonValue& value )
		{
			return value.isNull() || value.isUndefined();
		}

		bool matchArrays( const QJsonArray& array, const QJsonArray& expectedEntries )
		{
			if ( m_ensureArrayOrder )
				return matchArraysEnsureOrder( array, expectedEntries ); // RECURSION !!!
			return matchArraysIgnoreOrder( array, expectedEntries );     // RECURSION !!!
		}

		bool matchArraysIgnoreOrder( const QJsonArray& array, QJsonArray expectedEntries )
		{
			for ( auto&& entry : array )
			{
				auto expectedIter = expectedEntries.begin();
				const auto expectedEnd = expectedEntries.end();
				while ( expectedIter != expectedEnd )
				{
					if ( matchValues( entry, *expectedIter ) ) // RECURSION !!!
					{
						expectedIter = expectedEntries.erase( expectedIter );
						break;
					}

					++expectedIter;
				}
				if ( expectedEntries.isEmpty() )
					return true;
			}
			return false;
		}

		bool matchArraysEnsureOrder( const QJsonArray& array, QJsonArray expectedEntries )
		{
			auto expectedIter = expectedEntries.begin();

			for ( auto&& entry : array )
			{
				if ( matchValues( entry, *expectedIter ) ) // RECURSION !!!
				{
					expectedIter = expectedEntries.erase( expectedIter );
					if ( expectedEntries.isEmpty() )
						return true;
				}
			}
			return false;
		}

		bool matchObjects( const QJsonObject& object, const QJsonObject& expectedProps )
		{
			auto iter = expectedProps.constBegin();
			const auto end = expectedProps.constEnd();

			for ( ; iter != end; ++iter )
			{
				if ( ! object.contains( iter.key() )
				     || ! matchValues( object.value( iter.key() ), iter.value() ) ) // RECURSION !!!
					return false;
			}
			return true;
		}

		QJsonDocument m_bodyPart;
		bool m_ensureArrayOrder;
	};


} // namespace Predicates

} // namespace MockNetworkAccess

Q_DECLARE_METATYPE( MockNetworkAccess::VersionNumber )
Q_DECLARE_METATYPE( MockNetworkAccess::Request )
Q_DECLARE_METATYPE( MockNetworkAccess::Rule::Ptr )

namespace MockNetworkAccess {

/*! \internal Implementation details.
 */
namespace detail {

	/*! \internal
	 * Interface to access the Manager.
	 * The purpose of this interface is to give the NetworkReply access to the Manager without having
	 * a circular dependency.
	 */
	class ManagerInterface
	{
	public:
		virtual const QNetworkAccessManager* publicInterface() const = 0;
		virtual BehaviorFlags behaviorFlags() const = 0;
		virtual bool inspectBody() const = 0;
		virtual const AttributeSet& userDefinedAttributes() const = 0;
		virtual UnmatchedRequestBehavior unmatchedRequestBehavior() const = 0;
		virtual const MockReplyBuilder& unmatchedRequestBuilder() const = 0;
		virtual QNetworkAccessManager* forwardingTargetNam() const = 0;
		virtual QVector< Rule::Ptr > rules() const = 0;
		virtual void addHandledRequest( const Request& request ) = 0;
		virtual void addMatchedRequest( const Request& request, const Rule::Ptr& matchedRule ) = 0;
		virtual void addUnmatchedRequest( const Request& request ) = 0;
		virtual void addForwardedRequest( const Request& request ) = 0;
		virtual detail::ReplyHeaderHandler* replyHeaderHandler() const = 0;
		virtual QNetworkReply* createRequestBase( QNetworkAccessManager::Operation op,
		                                          const QNetworkRequest& originalReq,
		                                          QIODevice* outgoingData = nullptr ) = 0;
		virtual QAuthenticator getAuthenticator( QNetworkReply* unauthedReply,
		                                         const Request& unauthedReq,
		                                         const HttpUtils::Authentication::Challenge::Ptr& authChallenge ) = 0;
		virtual ConnectionPool& getConnectionPool() = 0;

	protected:
		~ManagerInterface() = default;
	};

	/*! \internal
	 * Checks if a redirect would cause a security degradation.
	 *
	 * \param from The URL from which the request is redirected.
	 * \param to The target URL of the redirect.
	 * \return \c true if a redirect from \p from to \p to degrades protocol security (that is, HTTPS to HTTP).
	 */
	inline bool secureToUnsecureRedirect( const QUrl& from, const QUrl& to )
	{
		return from.scheme().toLower() == HttpUtils::httpsScheme() && to.scheme().toLower() == HttpUtils::httpScheme();
	}

	/*! \internal
	 * Checks if two URLs refer to the same origin.
	 *
	 * \param left One QUrl to compare.
	 * \param right The other QUrl to compare.
	 * \return \c true if \p left and \p right refer to the same origin.
	 */
	inline bool isSameOrigin( const QUrl& left, const QUrl& right )
	{
		return left.scheme() == right.scheme() && left.host() == right.host() && left.port() == right.port();
	}

	/*! \internal
	 * Constructs a QBuffer containing the given data.
	 *
	 * \param data The data used to fill the buffer.
	 * \param inspectBody If \c false, this method returns a \c nullptr.
	 * \return A unique ptr to a QBuffer containing \p data unless \p data is null or \p inspectBody is false.
	 * The returned buffer is opened for reading (\c QIODevice::ReadOnly).
	 */
	inline std::unique_ptr< QIODevice > createIODeviceForUpload( const QByteArray& data, bool inspectBody )
	{
		if ( ! inspectBody || data.isNull() )
			return nullptr;

		auto buffer = std::unique_ptr< QBuffer >{ new QBuffer() };
		buffer->setData( data );
		buffer->open( QIODevice::ReadOnly );
		return buffer;
	}


} // namespace detail

/*! The QNetworkReply implementation which is returned by the Manager.
 * Basically, it is a wrapper class around either MockReply or QNetworkReply.
 * \since 0.11.0
 */
class NetworkReply : public CloneableNetworkReply
{
	Q_OBJECT

	friend class ManagerInterface;
	template< class Base >
	friend class Manager;

public:
	/*! QSharedPointer of QNetworkReply.
	 */
	using Ptr = QSharedPointer< QNetworkReply >;

	/*! Releases the connection.
	 * \sa QNetworkReply::~QNetworkReply()
	 */
	~NetworkReply() override
	{
		releaseConnection();
	}

	/*! Returns the valid attributes of a QNetworkReply.
	 *
	 * This method checks \p reply for the attributes up to MOCKNETWORKACCESS_MAX_PREDEFINED_ATTRIBUTE_ID
	 * as well as the \p additionalAttributes and returns a QHash of those attributes which have a valid
	 * value.
	 *
	 * \param reply The reply whose attributes should be returned.
	 * \param additionalAttributes Additional attributes to include in the check. By default, only the attributes up to
	 * MOCKNETWORKACCESS_MAX_PREDEFINED_ATTRIBUTE_ID are checked. To check for other attributes, provide them in this
	 * set.
	 * \return An AttributeHash of the valid (QVariant::isValid()) attributes of \p reply.
	 */
	static AttributeHash attributes( const QNetworkReply* reply, const AttributeSet& additionalAttributes = {} )
	{
		AttributeHash result;
		auto attributes = additionalAttributes;
		for ( AttributeIdType i = 0; i < MOCKNETWORKACCESS_MAX_PREDEFINED_ATTRIBUTE_ID; ++i )
			attributes << static_cast< QNetworkRequest::Attribute >( i );
		for ( auto&& attribute : attributes )
		{
			const auto attributeValue = reply->attribute( attribute );
			if ( attributeValue.isValid() )
				result.insert( attribute, attributeValue );
		}
		return result;
	}


	/*! Checks if a given reply indicates that the request requires authentication.
	 * \param reply The reply to be checked.
	 * \return \c true if the HTTP status code indicates that the request must be resend
	 * with appropriate authentication to succeed.
	 */
	static bool requiresAuthentication( const QNetworkReply* reply )
	{
		switch ( reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt() )
		{
			case HttpStatus::Unauthorized:                // 401
			case HttpStatus::ProxyAuthenticationRequired: // 407
				return true;
			default:
				return false;
		}
	}

	/*! \return The user defined attributes which were set on this NetworkReply.
	 * \sa setUserDefinedAttributes()
	 */
	AttributeSet userDefinedAttributes() const
	{
		return m_userDefinedAttributes;
	}

	/*! Sets additional QNetworkReply::Attribute to consider.
	 *
	 * When wrapping a regular QNetworkReply, the NetworkReply has to copy the attributes of the wrapped reply. Since
	 * there is no way to determine which attributes are set on a QNetworkReply, the NetworkReply normally only copies
	 * the attributes up to MOCKNETWORKACCESS_MAX_PREDEFINED_ATTRIBUTE_ID. If additional attributes should be copied,
	 * they must be registered using this method.
	 *
	 * \param userDefinedAttributes A QSet of additional attributes to copy when wrapping a QNetworkReply.
	 */
	void setUserDefinedAttributes( const AttributeSet& userDefinedAttributes )
	{
		m_userDefinedAttributes = userDefinedAttributes;
	}

	/*! \return \c true if the data of this NetworkReply has been read completely.
	 * \sa QIODevice::atEnd()
	 */
	bool atEnd() const override
	{
		Q_ASSERT( m_wrappedReply );
		return m_wrappedReply->atEnd();
	}

	/*! \return The number of bytes which can be read from this NetworkReply.
	 * \sa QIODevice::bytesAvailable()
	 */
	qint64 bytesAvailable() const override
	{
		Q_ASSERT( m_wrappedReply );
		return m_wrappedReply->bytesAvailable();
	}

	/*! \return 0
	 */
	qint64 bytesToWrite() const override
	{
		Q_ASSERT( m_wrappedReply );
		return m_wrappedReply->bytesToWrite();
	}

	/*! \return \c true if a line can be read from this NetworkReply.
	 * \sa QIODevice::canReadLine()
	 */
	bool canReadLine() const override
	{
		Q_ASSERT( m_wrappedReply );
		return m_wrappedReply->canReadLine();
	}

	/*! Closes the wrapped network reply.
	 * \sa QNetworkReply::close()
	 */
	void close() override
	{
		Q_ASSERT( m_wrappedReply );
		m_wrappedReply->close();
		QIODevice::close();
	}

	/*! \return The number of bytes which can be read from this NetworkReply.
	 * \sa QIODevice::size()
	 */
	qint64 size() const override
	{
		Q_ASSERT( m_wrappedReply );
		return m_wrappedReply->size();
	}

/*! Changes the open state of the reply.
 * \param mode The new mode.
 * \sa QIODevice::open()
 */
#if QT_VERSION < QT_VERSION_CHECK( 6,0,0 )
	bool open( QIODevice::OpenMode mode ) override
#else
	bool open( QIODeviceBase::OpenMode mode ) override
#endif // Qt >= 6.0.0
	{
		QIODevice::open( mode );
		Q_ASSERT( m_wrappedReply );
		const auto result = m_wrappedReply->open( mode );
		this->setOpenMode( m_wrappedReply->openMode() );
		return result;
	}

public Q_SLOTS:

	/*! Aborts the network communication.
	 * \sa QNetworkReply::abort()
	 */
	void abort() override
	{
		if ( m_wrappedReply )
		{
			m_wrappedReply->abort();
			copyPropertiesFromWrappedReply();
		}
		releaseConnection();
		setFinished( true );
	}

protected:
	/*! Creates a NetworkReply object.
	 * \param parent Parent QObject.
	 */
	explicit NetworkReply( QObject* parent = nullptr )
	    : CloneableNetworkReply( parent )
	{
	}

	/*! Sets the error state of this NetworkReply.
	 *
	 * \param code The error code.
	 * \param message An optional error message. If this is null (QString::isNull()),
	 * then %Qt's standard error message for the given \p code is used.
	 */
	void setError( QNetworkReply::NetworkError code, const QString& message = {} )
	{
		Q_ASSERT( m_wrappedReply );
		const auto reasonPhrase = m_wrappedReply->attribute( QNetworkRequest::HttpReasonPhraseAttribute ).toString();
		const auto errorString = message.isNull()
		                             ? detail::StandardErrorStringResolver( m_currentRequest, reasonPhrase ).resolve( code )
		                             : message;
		QNetworkReply::setError( code, errorString );
	}

	/*! Modifies the behavior of this NetworkReply.
	 *
	 * \param behaviorFlags Combination of BehaviorFlags to define some details of this NetworkReply's behavior.
	 * \note Only certain BehaviorFlags have an effect on a NetworkReply.
	 * \sa BehaviorFlag
	 */
	void setBehaviorFlags( BehaviorFlags behaviorFlags )
	{
		m_behaviorFlags = behaviorFlags;
	}

	/*! Sets given raw headers.
	 *
	 * Existing headers with the same name will be overridden. Other existing headers stay unchanged.
	 *
	 * \param rawHeaders A list of raw header pairs to be set for the reply.
	 */
	void updateRawHeaderPairs( const RawHeaderPairList& rawHeaders )
	{
		for ( auto&& headerPair : rawHeaders )
			this->setRawHeader( headerPair.first, headerPair.second );
	}

	/*! Reads response data.
	 *
	 * \param data The target buffer to write the data to.
	 * \param maxSize The maximum number of bytes to read.
	 * \return The number of bytes actually read.
	 * \sa QIODevice::readData()
	 */
	qint64 readData( char* data, qint64 maxSize ) override
	{
		Q_ASSERT( m_wrappedReply );
		return m_wrappedReply->read( data, maxSize );
	}

	// LCOV_EXCL_START
	/*! Effectively does nothing.
	 *
	 * \param data Ignored.
	 * \param maxSize Ignored.
	 * \return 0
	 */
	qint64 writeData( const char* data, qint64 maxSize ) override
	{
		Q_ASSERT( m_wrappedReply );
		return m_wrappedReply->write( data, maxSize );
	}
	// LCOV_EXCL_STOP

#if QT_VERSION >= QT_VERSION_CHECK( 6,0,0 )
	/*! Skips data from reading.
	 *
	 * \param maxSize The maximum number of bytes to skip.
	 * \return The number of bytes actually skipped.
	 * \sa QIODevice::skipData()
	 */
	qint64 skipData( qint64 maxSize ) override
	{
		Q_ASSERT( m_wrappedReply );
		return m_wrappedReply->skip( maxSize );
	}
#endif // Qt >= 6.0.0

private:
	void run( detail::ManagerInterface* manager, const Request& request )
	{
		Q_ASSERT( manager );
		Q_ASSERT_X( ! m_manager, Q_FUNC_INFO, "run() was called on an already running NetworkReply" );

		m_manager = manager;
		m_rules = m_manager->rules();               // shallow copy in case some rules are stateful
		this->copyPropertiesFromRequest( request ); // only the properties of the initial request are copied
		connectReplyHeaderHandler();
		handleRequest( request );
	}

	void connectReplyHeaderHandler()
	{
		QObject::connect( this, &QNetworkReply::metaDataChanged, m_manager->replyHeaderHandler(), [ this ]() {
			m_manager->replyHeaderHandler()->handleReplyHeaders( this );
		} );
	}

	void handleRequest( const Request& request )
	{
		Q_ASSERT( m_manager );

		m_currentRequest = request;
		releaseConnection();
		acquireNewConnection();
		m_manager->addHandledRequest( request );
		clearWrappedReply();

		if ( m_currentConnection->isConnected() )
			handleConnected();
		else
			QObject::connect( m_currentConnection, &detail::Connection::connected, this, &NetworkReply::handleConnected );
	}

	void releaseConnection()
	{
		if ( m_currentConnection )
		{
			QObject::disconnect( m_currentConnection, nullptr, this, nullptr );
			m_currentConnection->release();
			m_currentConnection = nullptr;
		}
	}

	void acquireNewConnection()
	{
		Q_ASSERT( m_manager );
		m_currentConnection = m_manager->getConnectionPool().getConnection( m_currentRequest.qRequest.url() );
	}

private Q_SLOTS:

	void handleConnected()
	{
		std::unique_ptr< MockReply > mockedReply;
		auto ruleIter = m_rules.cbegin();
		const auto rulesEnd = m_rules.cend();
		for ( ; ruleIter != rulesEnd; ++ruleIter )
		{
			const auto rule = *ruleIter;
			if ( ! rule->matches( m_currentRequest ) )
				continue;

			if ( rule->forwardingBehavior() != Rule::ForwardAndReturnDelegatedReply )
			{
				mockedReply = rule->createReply( m_currentRequest );
				if ( ! mockedReply )
					continue;
			}

			break;
		}

		if ( ruleIter == rulesEnd )
		{
			m_manager->addUnmatchedRequest( m_currentRequest );
			handleUnmatchedRequest();
		}
		else
		{
			const auto matchedRule = *ruleIter;
			m_manager->addMatchedRequest( m_currentRequest, matchedRule );
			handleMatchedRequest( matchedRule, std::move( mockedReply ) );
		}

		handleWrappedReply();
	}


private:
	std::unique_ptr< QNetworkReply > forwardRequest( QNetworkAccessManager* overrideForwardingNam = nullptr )
	{
		m_manager->addForwardedRequest( m_currentRequest );

		auto ioDevice = detail::createIODeviceForUpload( m_currentRequest.body, m_manager->inspectBody() );

		auto forwardingRequest = m_currentRequest;

// Disable automatic redirect following and handle it manually
#if QT_VERSION >= QT_VERSION_CHECK( 5,9,0 )
		forwardingRequest.qRequest.setAttribute( QNetworkRequest::RedirectPolicyAttribute,
		                                         static_cast< int >( QNetworkRequest::ManualRedirectPolicy ) );
#else
		forwardingRequest.qRequest.setAttribute( QNetworkRequest::FollowRedirectsAttribute, false );
#endif

		auto* forwardingNam = overrideForwardingNam ? overrideForwardingNam : m_manager->forwardingTargetNam();

		std::unique_ptr< QNetworkReply > reply;
		if ( forwardingNam )
		{
			switch ( forwardingRequest.operation )
			{
				case QNetworkAccessManager::GetOperation:
					reply.reset( forwardingNam->get( forwardingRequest.qRequest ) );
					break;
				case QNetworkAccessManager::PostOperation:
					reply.reset( forwardingNam->post( forwardingRequest.qRequest, ioDevice.get() ) );
					break;
				case QNetworkAccessManager::PutOperation:
					reply.reset( forwardingNam->put( forwardingRequest.qRequest, ioDevice.get() ) );
					break;
				case QNetworkAccessManager::HeadOperation:
					reply.reset( forwardingNam->head( forwardingRequest.qRequest ) );
					break;
				case QNetworkAccessManager::DeleteOperation:
					reply.reset( forwardingNam->deleteResource( forwardingRequest.qRequest ) );
					break;
				case QNetworkAccessManager::CustomOperation:
				default:
					reply.reset( forwardingNam->sendCustomRequest(
					    forwardingRequest.qRequest,
					    forwardingRequest.qRequest.attribute( QNetworkRequest::CustomVerbAttribute ).toByteArray(),
					    ioDevice.get() ) );
					break;
			}
		}
		else
			reply.reset(
			    m_manager->createRequestBase( forwardingRequest.operation, forwardingRequest.qRequest, ioDevice.get() ) );

		if ( ioDevice )
		{
			QObject::connect( reply.get(), &QNetworkReply::finished, ioDevice.get(), &QObject::deleteLater );
			ioDevice.release()->setParent( reply.get() );
		}
		auto* rawReply = reply.get();
		QObject::connect( rawReply,
		                  &QNetworkReply::metaDataChanged,
		                  m_manager->replyHeaderHandler(),
		                  [ this, rawReply ]() { m_manager->replyHeaderHandler()->handleReplyHeaders( rawReply ); } );
		return reply;
	}

	bool authenticateRequest()
	{
		const auto unauthedReq = m_currentRequest;
		auto authChallenges = HttpUtils::Authentication::getAuthenticationChallenges( m_wrappedReply );

		if ( authChallenges.isEmpty() )
		{
			qCWarning( log ) << "Cannot authenticate request because there is no supported authentication challenge in"
			                 << "reply from URL" << m_wrappedReply->url().toString();
			return false;
		}

		/* Select the strongest challenge.
		 * If there are multiple challenges with the same strength,
		 * the last one is used according to the order they appear in the HTTP headers.
		 */
		std::stable_sort( authChallenges.begin(),
		                  authChallenges.end(),
		                  HttpUtils::Authentication::Challenge::StrengthCompare() );
		auto authChallenge = authChallenges.last();

		auto authenticator = m_manager->getAuthenticator( m_wrappedReply, unauthedReq, authChallenge );
		if ( authenticator.user().isNull() && authenticator.password().isNull() )
			return false;

		QNetworkRequest authedQReq( unauthedReq.qRequest );
		authChallenge->addAuthorization( authedQReq, unauthedReq.operation, unauthedReq.body, authenticator );
		const Request authedReq( unauthedReq.operation, authedQReq, unauthedReq.body );
		this->handleRequest( authedReq ); // Start over with a new request!
		return true;
	}

	bool decideAboutRedirect()
	{
		const auto prevTarget = m_currentRequest.qRequest.url();
		const auto nextTarget = prevTarget.resolved( NetworkReplyUtils::locationHeader( m_wrappedReply ) );
		const auto nextTargetScheme = nextTarget.scheme().toLower();

#if QT_VERSION < QT_VERSION_CHECK( 5,15,2 )
		const auto followRedirectsAttr = m_currentRequest.qRequest.attribute( QNetworkRequest::FollowRedirectsAttribute );
#endif // Qt < 5.15.2
#if QT_VERSION >= QT_VERSION_CHECK( 5,9,0 )
		// Redirect policy takes precedence over FollowRedirectsAttribute
		const auto redirectPolicyAttr = m_currentRequest.qRequest.attribute( QNetworkRequest::RedirectPolicyAttribute );
		if ( redirectPolicyAttr.isValid() )
		{
			const auto redirectPolicy = static_cast< QNetworkRequest::RedirectPolicy >( redirectPolicyAttr.toInt() );
			if ( ! applyRedirectPolicy( redirectPolicy, nextTarget ) )
				return false;
		}
		else
#endif // Qt >= 5.9.0
#if QT_VERSION < QT_VERSION_CHECK( 5,15,2 )
		    if ( followRedirectsAttr.isValid() )
		{
			if ( ! followRedirectsAttr.toBool() )
				return false;

			if ( ! nextTarget.isValid() )
			{
				this->setProtocolUnknownError();
				return false;
			}

			if ( detail::secureToUnsecureRedirect( prevTarget, nextTarget ) )
			{
				setError( QNetworkReply::InsecureRedirectError );
				return false;
			}
		}
		else
#endif // Qt < 5.15.2
		{
#if QT_VERSION >= QT_VERSION_CHECK( 5,9,0 )
			if ( ! applyRedirectPolicy( m_manager->publicInterface()->redirectPolicy(), nextTarget ) )
				return false;
#else  // Qt < 5.9.0
       // Following the redirect is not requested
			return false;
#endif // Qt >= 5.9.0
		}

		if ( ! nextTarget.isValid()
		     || ( nextTargetScheme != HttpUtils::httpScheme() && nextTargetScheme != HttpUtils::httpsScheme() ) )
		{
			this->setProtocolUnknownError();
			return false;
		}

		if ( m_currentRequest.qRequest.maximumRedirectsAllowed() <= 0 )
		{
			this->setError( QNetworkReply::TooManyRedirectsError );
			return false;
		}

		return true;
	}

	void followRedirect()
	{
		const auto nextReq = prepareRedirectRequest();

		Q_EMIT this->redirected( nextReq.qRequest.url() );

		this->handleRequest( nextReq ); // Start over with a new request!
	}

	Request prepareRedirectRequest() const
	{
		const auto prevTarget = m_currentRequest.qRequest.url();
		const auto nextTarget = prevTarget.resolved( NetworkReplyUtils::locationHeader( m_wrappedReply ) );
		const auto statusCodeAttr = m_wrappedReply->attribute( QNetworkRequest::HttpStatusCodeAttribute );

		QNetworkAccessManager::Operation nextOperation;
		QByteArray nextReqBody;
		if ( m_currentRequest.operation == QNetworkAccessManager::GetOperation
		     || m_currentRequest.operation == QNetworkAccessManager::HeadOperation )
			nextOperation = m_currentRequest.operation;
		else if ( m_behaviorFlags.testFlag( Behavior_RedirectWithGet ) )
			// Qt up to 5.9.3 always redirects with a GET
			nextOperation = QNetworkAccessManager::GetOperation;
		else
		{
			nextOperation = m_currentRequest.operation;
			nextReqBody = m_currentRequest.body;

			switch ( static_cast< HttpStatus::Code >( statusCodeAttr.toInt() ) )
			{
				case HttpStatus::TemporaryRedirect: // 307
				case HttpStatus::PermanentRedirect: // 308
					break;
				case HttpStatus::MovedPermanently: // 301
				case HttpStatus::Found:            // 302
				{
					/* This is the behavior of most browsers and clients:
					 * RFC-7231 defines the request methods GET, HEAD, OPTIONS, and TRACE to be safe
					 * for automatic redirection. GET and HEAD are already handled above.
					 * See https://tools.ietf.org/html/rfc7231#section-6.4
					 * and https://tools.ietf.org/html/rfc7231#section-4.2.1
					 */
					const auto customVerb = m_currentRequest.qRequest.attribute( QNetworkRequest::CustomVerbAttribute )
					                            .toByteArray()
					                            .toLower();
					if ( ! m_behaviorFlags.testFlag( Behavior_IgnoreSafeRedirectMethods )
					     && m_currentRequest.operation == QNetworkAccessManager::CustomOperation
					     && ( customVerb == QByteArrayLiteral( "options" ) || customVerb == QByteArrayLiteral( "trace" ) ) )
					{
						break;
					}
					Q_FALLTHROUGH();
				}
				case HttpStatus::SeeOther: // 303
				default:
					nextOperation = QNetworkAccessManager::GetOperation;
					nextReqBody.clear();
					break;
			}
		}

		auto nextQReq = m_currentRequest.qRequest;
		nextQReq.setUrl( nextTarget );
		nextQReq.setMaximumRedirectsAllowed( m_currentRequest.qRequest.maximumRedirectsAllowed() - 1 );
		if ( nextOperation != QNetworkAccessManager::CustomOperation )
			nextQReq.setAttribute( QNetworkRequest::CustomVerbAttribute, QVariant() );

		return Request( nextOperation, nextQReq, nextReqBody );
	}

	void setProtocolUnknownError()
	{
		setError( QNetworkReply::ProtocolUnknownError );
		setAttribute( QNetworkRequest::RedirectionTargetAttribute, QVariant() );
	}

#if QT_VERSION >= QT_VERSION_CHECK( 5,9,0 )

	bool applyRedirectPolicy( QNetworkRequest::RedirectPolicy policy, const QUrl& redirectTarget )
	{
		if ( policy == QNetworkRequest::ManualRedirectPolicy )
			return false;

		if ( ! redirectTarget.isValid() )
		{
			this->setProtocolUnknownError();
			return false;
		}

		const auto prevTarget = m_currentRequest.qRequest.url();

		switch ( policy )
		{
			case QNetworkRequest::NoLessSafeRedirectPolicy:
				if ( detail::secureToUnsecureRedirect( prevTarget, redirectTarget ) )
				{
					this->setError( QNetworkReply::InsecureRedirectError );
					return false;
				}
				break;
			case QNetworkRequest::SameOriginRedirectPolicy:
				if ( ! detail::isSameOrigin( prevTarget, redirectTarget ) )
				{
					this->setError( QNetworkReply::InsecureRedirectError );
					return false;
				}
				break;
			case QNetworkRequest::UserVerifiedRedirectPolicy:
				// TODO: QNetworkRequest::UserVerifiedRedirectPolicy
				qCWarning( log ) << "User verified redirection policy is not supported at the moment";
				this->setError( QNetworkReply::InsecureRedirectError );
				return false;
				break;
			// LCOV_EXCL_START
			default:
				qCWarning( log ) << "Unknown redirect policy:" << policy;
				this->setError( QNetworkReply::InsecureRedirectError );
				return false;
				// LCOV_EXCL_STOP
		}

		return true;
	}

#endif // Qt >= 5.9.0

	void clearWrappedReply()
	{
		if ( m_wrappedReply )
		{
			disconnectWrappedReplySignals();
			m_wrappedReply->abort();
			/* When handling authentication or redirection, this is called in reaction to a metaDataChanged signal
			 * of the wrapped reply. So we must not delete it here directly but delete it later.
			 */
			m_wrappedReply->deleteLater();
			m_wrappedReply = nullptr;
			m_wrappedMockReply.release();
			m_wrappedForwardedReply.release();
		}
	}

	void handleUnmatchedRequest()
	{
		switch ( m_manager->unmatchedRequestBehavior() )
		{
			case PredefinedReply:
				wrapMockedReply( std::unique_ptr< MockReply >( m_manager->unmatchedRequestBuilder().createReply() ) );
				break;
			case Forward:
				wrapReplyOfForwardedRequest( forwardRequest() );
				break;
		}
	}

	void handleMatchedRequest( const Rule::Ptr& matchedRule, std::unique_ptr< MockReply > mockedReply )
	{
		if ( matchedRule->forwardingBehavior() != Rule::DontForward )
		{
			auto forwardedReply = forwardRequest( matchedRule->forwardingTargetManager() );
			switch ( matchedRule->forwardingBehavior() )
			{
				case Rule::ForwardAndReturnMockReply:
					QObject::connect( forwardedReply.get(),
					                  &QNetworkReply::finished,
					                  forwardedReply.get(),
					                  &QObject::deleteLater );
					forwardedReply.release()->setParent( this );
					break;
				case Rule::ForwardAndReturnDelegatedReply:
					wrapReplyOfForwardedRequest( std::move( forwardedReply ) );
					return;
				// LCOV_EXCL_START
				case Rule::DontForward:
					Q_ASSERT_X( false, Q_FUNC_INFO, "Unexpected Rule::ForwardingBehavior" );
					break;
					// LCOV_EXCL_STOP
			}
		}

		wrapMockedReply( std::move( mockedReply ) );
	}

	void handleWrappedReply()
	{
		if ( m_wrappedForwardedReply )
			return;

		Q_ASSERT( m_wrappedMockReply );
		m_wrappedMockReply->setBehaviorFlags( m_behaviorFlags );
		m_wrappedMockReply->simulate( m_currentRequest, m_currentConnection );
	}

	void wrapMockedReply( std::unique_ptr< MockReply > mockReply )
	{
		m_wrappedMockReply = std::move( mockReply );
		m_wrappedReply = m_wrappedMockReply.get();
		connectWrappedReplySignals();
	}

	void wrapReplyOfForwardedRequest( std::unique_ptr< QNetworkReply > reply )
	{
		m_wrappedForwardedReply = std::move( reply );
		m_wrappedReply = m_wrappedForwardedReply.get();
		connectWrappedReplySignals();
	}

	void connectWrappedReplySignals()
	{
		// QIODevice signals
		QObject::connect( m_wrappedReply, &QNetworkReply::aboutToClose, this, &QNetworkReply::aboutToClose );
		QObject::connect( m_wrappedReply, &QNetworkReply::bytesWritten, this, &QNetworkReply::bytesWritten );
		QObject::connect( m_wrappedReply, &QNetworkReply::readChannelFinished, this, &QNetworkReply::readChannelFinished );
		QObject::connect( m_wrappedReply, &QNetworkReply::readyRead, this, &QNetworkReply::readyRead );
#if QT_VERSION >= QT_VERSION_CHECK( 5,7,0 )
		QObject::connect( m_wrappedReply, &QNetworkReply::channelBytesWritten, this, &QNetworkReply::channelBytesWritten );
		QObject::connect( m_wrappedReply, &QNetworkReply::channelReadyRead, this, &QNetworkReply::channelReadyRead );
#endif

		// QNetworkReply signals
		QObject::connect( m_wrappedReply, &QNetworkReply::downloadProgress, this, &QNetworkReply::downloadProgress );
		QObject::connect( m_wrappedReply, &QNetworkReply::encrypted, this, &QNetworkReply::encrypted, Qt::DirectConnection );
#if QT_VERSION < QT_VERSION_CHECK( 5,15,0 )
		QObject::connect( m_wrappedReply,
		                  SIGNAL( error( QNetworkReply::NetworkError ) ),
		                  this,
		                  SIGNAL( error( QNetworkReply::NetworkError ) ) );
#else  // Qt >= 5.15.0
		QObject::connect( m_wrappedReply, &QNetworkReply::errorOccurred, this, &QNetworkReply::errorOccurred );
#endif // Qt >= 5.15.0
		QObject::connect( m_wrappedReply, &QNetworkReply::finished, this, &NetworkReply::handleFinished );
		QObject::connect( m_wrappedReply, &QNetworkReply::metaDataChanged, this, &NetworkReply::handleMetaDataChanged );
		QObject::connect( m_wrappedReply, &QNetworkReply::sslErrors, this, &QNetworkReply::sslErrors, Qt::DirectConnection );
		QObject::connect( m_wrappedReply, &QNetworkReply::uploadProgress, this, &QNetworkReply::uploadProgress );
		QObject::connect( m_wrappedReply,
		                  &QNetworkReply::preSharedKeyAuthenticationRequired,
		                  this,
		                  &QNetworkReply::preSharedKeyAuthenticationRequired,
		                  Qt::DirectConnection );

		/* Redirection signals won't be emitted because forwarded replies are set to not follow redirects automatically
		 * and mocked replies don't emit them.
		 * Therefore, we don't need to connect to
		 * - QNetworkReply::redirected
		 * - QNetworkReply::redirectAllowed
		 */
	}

	void disconnectWrappedReplySignals()
	{
		m_wrappedReply->disconnect( this );
		this->disconnect( m_wrappedReply );
	}

	void copyPropertiesFromWrappedReply()
	{
		this->setOpenMode( m_wrappedReply->openMode() );
		CloneableNetworkReply::copyReplyPropertiesFrom( m_wrappedReply, m_userDefinedAttributes );
	}

	void concatenateRawHeaders()
	{
		if ( ! m_wasRedirected || ! m_behaviorFlags.testFlag( Behavior_ConcatRawHeadersAfterRedirect ) )
			return;

		const auto concatenatedHeaders = buildConcatenatedRawHeaders( m_wrappedReply->rawHeaderPairs() );
		updateRawHeaderPairs( detail::rawHeaderHashToPairList( concatenatedHeaders ) );
		m_previousRawHeaderValues = concatenatedHeaders;
	}

	RawHeaderHash buildConcatenatedRawHeaders( const RawHeaderPairList& newRawHeaders ) const
	{
		auto result = m_previousRawHeaderValues;

		for ( auto&& newRawHeader : newRawHeaders )
		{
			const auto& headerKey = newRawHeader.first;
			const auto& newHeaderValue = newRawHeader.second;
			if ( headerKey == HttpUtils::locationHeader() )
				result.insert( headerKey, newHeaderValue );
			else
			{
				const auto previousValue = m_previousRawHeaderValues.value( headerKey );
				if ( previousValue.isEmpty() )
					result.insert( headerKey, newHeaderValue );
				else if ( ! newHeaderValue.isEmpty() )
					result.insert( headerKey, previousValue + QByteArrayLiteral( ", " ) + newHeaderValue );
			}
		}

		return result;
	}

private Q_SLOTS:
	void handleMetaDataChanged()
	{
		updateConnectionSettings();

		if ( NetworkReply::requiresAuthentication( m_wrappedReply ) )
		{
			const auto authenticated = this->authenticateRequest();
			if ( authenticated )
			{
				/* No metaDataChanged() is emitted in this case!
				 * Authentication happens "silently".
				 */
				// !!! We are now handling a new request !!!
				return;
			}
		}

		if ( NetworkReplyUtils::isRedirectToBeFollowed( m_wrappedReply, m_behaviorFlags ) )
		{
			copyPropertiesFromWrappedReply();

			if ( decideAboutRedirect() ) // Should and can we actually redirect?
			{
				m_wasRedirected = true;
				concatenateRawHeaders();
				Q_EMIT this->metaDataChanged();

				followRedirect();
				// !!! We are now handling a new request !!!
				return;
			}

			Q_EMIT this->metaDataChanged();
			finishWithRedirectionReply();
			return;
		}

		// "Regular" reply
		mirrorWrappedReply();
		Q_EMIT this->metaDataChanged();
	}

private:
	void updateConnectionSettings()
	{
		Q_ASSERT( m_wrappedReply );
		/* QNetworkAccessManager ignores the Keep-Alive header so this does the same.
		 */
		const bool keepAlive = m_wrappedReply->rawHeader( HttpUtils::connectionHeader() ).toLower()
		                       != QByteArrayLiteral( "close" );
		m_currentConnection->setKeepAlive( keepAlive );
	}

	void mirrorWrappedReply()
	{
		copyPropertiesFromWrappedReply();
		concatenateRawHeaders();
	}

private Q_SLOTS:
	void handleFinished()
	{
		releaseConnection();
		this->setFinished( true );
		Q_EMIT this->finished();
	}

private:
	void finishWithRedirectionReply()
	{
		emitErrorSignalIfError();
		// "finish" happens when wrapped reply finishes
	}

	void emitErrorSignalIfError()
	{
		if ( this->error() != QNetworkReply::NoError )
		{
#if QT_VERSION < QT_VERSION_CHECK( 5,15,0 )
			Q_EMIT this->error( this->error() );
#else
			Q_EMIT this->errorOccurred( this->error() );
#endif
		}
	}


protected:
	/*! \return Returns the wrapped reply which is either a regular QNetworkReply or a MockedReply.
	 * The ownership stays with the NetworkReply.
	 */
	QNetworkReply* wrappedReply() const
	{
		return m_wrappedReply;
	}

private:
	BehaviorFlags m_behaviorFlags = Behavior_Expected;
	detail::ManagerInterface* m_manager = nullptr;
	QVector< Rule::Ptr > m_rules;
	std::unique_ptr< MockReply > m_wrappedMockReply;
	std::unique_ptr< QNetworkReply > m_wrappedForwardedReply;
	QNetworkReply* m_wrappedReply = nullptr;
	AttributeSet m_userDefinedAttributes;
	Request m_currentRequest;
	detail::Connection* m_currentConnection = nullptr;
	RawHeaderHash m_previousRawHeaderValues;
	bool m_didUpload = false;
	bool m_wasRedirected = false;
};


/*! Helper class which emits signals for the Manager.
 *
 * Since template classes cannot use the `Q_OBJECT` macro, they cannot define signals or slots.
 * For this reason, this helper class is needed to allow emitting signals from the Manager.
 *
 * To get the signal emitter, call Manager::signalEmitter().
 *
 * \sa Manager::signalEmitter()
 */
class SignalEmitter : public QObject
{
	Q_OBJECT

	template< class Base >
	friend class Manager;

public:
	/*! Default destructor
	 */
	~SignalEmitter() override = default;

private:
	/*! Creates a SignalEmitter object.
	 *
	 * \note This registers the types Request and Rule::Ptr in the %Qt meta type system
	 * using qRegisterMetaType().
	 *
	 * \param parent Parent QObject.
	 */
	explicit SignalEmitter( QObject* parent = Q_NULLPTR )
	    : QObject( parent )
	{
		registerMetaTypes();
	}

Q_SIGNALS:

	/*! Emitted right after the Manager received a request through its public interface (QNetworkAccessManager::get()
	 * etc.). \param request The request.
	 */
	void receivedRequest( const MockNetworkAccess::Request& request );

	/*! Emitted when the Manager starts handling a request.
	 *
	 * This signal is emitted for requests received through the public interface (see receivedRequest()) as well as
	 * requests created internally for example when automatically following redirects or when handling authentication.
	 *
	 * \param request The request.
	 */
	void handledRequest( const MockNetworkAccess::Request& request );

	/*! Emitted when a request matched a Rule.
	 * \param request The request.
	 * \param rule The matched Rule.
	 */
	void matchedRequest( const MockNetworkAccess::Request& request, MockNetworkAccess::Rule::Ptr rule );

	/*! Emitted when a request did not match any of the Manager's Rules.
	 * \param request The request.
	 */
	void unmatchedRequest( const MockNetworkAccess::Request& request );

	/*! Emitted before a request is forwarded to the next network access manager.
	 * \param request The request.
	 * \sa Manager::setForwardingTargetNam()
	 */
	void forwardedRequest( const MockNetworkAccess::Request& request );

private:
	static void registerMetaTypes()
	{
		static QAtomicInt registered;
		if ( registered.testAndSetAcquire( 0, 1 ) )
		{
			::qRegisterMetaType< Request >();
			::qRegisterMetaType< Rule::Ptr >();
		}
	}
};

/*! Mixin class to mock network replies from QNetworkAccessManager.
 * %Manager mocks the QNetworkReplys instead of sending the requests over the network.
 * %Manager is a mixin class meaning it can be used "on top" of every class inheriting publicly from
 * QNetworkAccessManager.
 *
 * \tparam Base QNetworkAccessManager or a class publicly derived from QNetworkAccessManager.
 *
 *
 * ## Configuration ##
 * To define which and how requests are answered with mocked replies, the %Manager is configured using
 * \link Rule Rules\endlink:
 * Whenever the %Manager is handed over a request, it matches the request against its rules one after the other.\n
 * - If a rule reports a match for the request, the %Manager requests the rule to create a reply for that request.\n
 *   - If the rule creates a reply, then this reply is returned by the %Manager.\n
 *   - If the rule does not create a reply, the %Manager continues matching the request against the remaining rules.\n
 * - If no rule matches the request or no rule created a reply, the "unmatched request behavior" steps in.\n
 *   This means either:
 *   1. the request is forwarded to the next network access manager (see setForwardingTargetNam()) and the corresponding
 *      QNetworkReply is returned.
 *   2. a predefined reply is returned (see unmatchedRequestBuilder()).
 *
 *   The latter is the default behavior. For more details see \ref UnmatchedRequestBehavior.
 *
 * To define which requests match a rule, the Rule object is configured by adding predicates.
 *
 * To define the properties of the created replies, the %Rule object exposes a MockReplyBuilder via the Rule::reply()
 * method.
 *
 * To add a rule to the %Manager, you can either:
 * - create a %Rule object, configure it and add it using addRule().
 * - use the convenience methods whenGet(), whenPost(), when() etc. and configure the returned %Rule objects.
 *
 * To retrieve or remove Rules or change their order, use the methods rules() and setRules().
 *
 *
 * ### Example ###
 *
 * \code
 * using namespace MockNetworkAccess;
 * using namespace MockNetworkAccess::Predicates;
 *
 * // Create the Manager
 * Manager< QNetworkAccessManager > mockNam;
 *
 * // Simple configuration
 * mockNam.whenGet( QRegularExpression( "https?://example.com/data/.*" ) )
 *        .reply().withBody( QJsonDocument::fromJson( "{ \"id\": 736184, \"data\": \"Hello World!\" }" );
 *
 * // More complex configuration
 * Rule::Ptr accountInfoRequest( new Rule );
 * accountInfoRequest->has( Verb( QNetworkAccessManager::GetOperation ) )
 *                   .has( UrlMatching( QRegularExpression( "https?://example.com/accountInfo/.*" ) ) );
 *
 * Rule::Ptr authorizedAccountInfoRequest( accountInfoRequest->clone() );
 * authorizedAccountInfoRequest->has( RawHeaderMatching( HttpUtils::authorizationHeader(), QRegularExpression( "Bearer:
 * .*" ) ) ).reply().withBody( QJsonDocument::fromJson( "{ \"name\": \"John Doe\", \"email\": \"john.doe@example.com\"
 * }" ) );
 *
 * Rule::Ptr unauthorizedAccountInfoRequest( accountInfoRequest->clone() );
 * unauthorizedAccountInfoRequest->reply().withStatus( 401 );
 *
 * // The order is important here since the
 * // first matching rule will create the reply.
 * mockNam.add( authorizedAccountInfoRequest );
 * mockNam.add( unauthorizedAccountInfoRequest );
 *
 * // All other requests
 * mockNam.unmatchedRequestBuilder().withStatus( 404 );
 *
 * // Use the Manager
 * MyNetworkClient myNetworkClient;
 * myNetworkClient.setNetworkManager( &mockNam );
 * myNetworkClient.run();
 * \endcode
 *
 * ### Signals ###
 * Since the Manager is a template class, it cannot define signals due to limitations of %Qt's meta object compiler
 * (moc).
 *
 * To solve this, the Manager provides a SignalEmitter (see signalEmitter()) which emits the signals on behalf of the
 * Manager.
 *
 * [QNetworkRequest::UserVerifiedRedirectPolicy]: http://doc.qt.io/qt-5/qnetworkrequest.html#RedirectPolicy-enum
 * [QNetworkRequest::Attributes]: http://doc.qt.io/qt-5/qnetworkrequest.html#Attribute-enum
 *
 *
 * ## Handling of non-HTTP Protocols ##
 * The Manager also supports FTP, `data:`, `file:` and `qrc:` requests. However, for `data:`, `file:` and `qrc:`
 * requests the Manager behaves differently as for HTTP or FTP requests.
 *
 * ### `data:` Requests ###
 * `data:` requests are always forwarded to the \p Base network access manager. That's the easiest way to implement the
 * handling of such requests and since they are never sent to the network it does not make sense to allow any kind of
 * reply mocking there. This means that requests with a `data:` URL are never matched against any rule and these
 * requests are never contained in the matchedRequests(), unmatchedRequests() or forwardedRequests(). However, they are
 * contained in the receivedRequests() and handledRequests().
 *
 * ### `file:` and `qrc:` Requests ###
 * Requests with a `file:` URL only support the \link QNetworkAccessManager::get() GET \endlink and
 * \link QNetworkAccessManager::put() PUT \endlink operations. Requests with a `qrc:` URL only support the
 * \link QNetworkAccessManager::get() GET \endlink operation. All other operations will result in a reply
 * with an QNetworkReply::ProtocolUnknownError.
 *
 * If you want to mock a successful `PUT` operation of a `file:` request, you should configure the rule to reply with
 * QNetworkReply::NoError. It is necessary to call one of the `with*()` methods of the MockReplyBuilder for the Rule
 * to be considered valid by the Manager. And setting `withError( QNetworkReply::NoError )` is the only configuration
 * that is applicable for a successful `PUT` operation for a `file:` request. For example:
 *
 * \code
 * using namespace MockNetworkAccess;
 *
 * Manager< QNetworkAccessManager > mnam;
 * mnam.whenPut( QUrl( "file:///path/to/file" ) ).reply().withError( QNetworkReply::NoError );
 * \endcode
 *
 *
 * ## Limitations ##
 * The Manager currently has a few limitations:
 * - The mocked replies do not emit the implementation specific signals of a real HTTP based QNetworkReply
 *   (that is the signals of QNetworkReplyHttpImpl).
 * - Out of the box, only HTTP Basic authentication is supported. However, this should not be a problem in most cases
 *   since the handling of authentication is normally done internally between the `MockNetworkAccess::Manager` and the
 *   `MockReply`.\n
 *   This is only a limitation if you manually create `Authorization` headers and have to rely on HTTP Digest or NTLM
 *   authentication.\n
 *   Note that it is still possible to work with any authentication method by matching the `Authorization` header
 *   manually (for example using Predicates::RawHeaderMatching) or by implementing a
 *   \link Rule::Predicate custom predicate\endlink.
 * - Proxy authentication is not supported at the moment.
 * - [QNetworkRequest::UserVerifiedRedirectPolicy] is not supported at the moment.
 * - The error messages of the replies (QNetworkReply::errorString()) may be different from the ones of real
 *   QNetworkReply objects.
 * - QNetworkReply::setReadBufferSize() is ignored at the moment.
 *
 *
 * Some of these limitations might be removed in future versions. Feel free to create a feature (or merge) request if
 * you hit one these limitations.
 *
 * Additionally, the Manager supports only selected [QNetworkRequest::Attributes].
 * The following attributes are supported:
 * - QNetworkRequest::HttpStatusCodeAttribute
 * - QNetworkRequest::HttpReasonPhraseAttribute
 * - QNetworkRequest::RedirectionTargetAttribute
 * - QNetworkRequest::ConnectionEncryptedAttribute
 * - QNetworkRequest::CustomVerbAttribute
 * - QNetworkRequest::CookieLoadControlAttribute
 * - QNetworkRequest::CookieSaveControlAttribute
 * - QNetworkRequest::FollowRedirectsAttribute
 * - QNetworkRequest::OriginalContentLengthAttribute
 * - QNetworkRequest::RedirectPolicyAttribute
 *
 * All other attributes are ignored when specified on a QNetworkRequest and are not set when returning a MockReply.
 * However, if desired, the attributes can be matched on a request using Predicates::Attribute or
 * Predicates::AttributeMatching and can be set on a MockReply using MockReplyBuilder::withAttribute().
 *
 * \note
 * \parblock
 * At the moment, the Manager does not handle large request bodies well since it reads them into
 * memory completely to be able to provide them to all the Rule objects.
 *
 * With setInspectBody(), you can disable this if you need to use the Manager with large request
 * bodies and you do not need to match against the body.
 * \endparblock
 *
 */
template< class Base >
class Manager
    : public Base
    , private detail::ManagerInterface
{
	// cannot use Q_OBJECT with template class
public:
	/*! Creates a Manager.
	 * \param parent Parent QObject.
	 */
	explicit Manager( QObject* parent = Q_NULLPTR )
	    : Base( parent )
	    , m_inspectBody( true )
	    , m_behaviorFlags( getDefaultBehaviorFlags() )
	    , m_forwardingTargetNam( Q_NULLPTR )
	    , m_signalEmitter( Q_NULLPTR )
	    , m_unmatchedRequestBehavior( PredefinedReply )
	    , m_replyHeaderHandler( new detail::ReplyHeaderHandler( this ) )
	    , m_ipAddressRegEx(
	          QStringLiteral( "^\\[.*\\]$|" // IPv6 addresses have to be enclosed in brackets
	                          "^((25[0-5]|2[0-4][0-9]|1?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|1?[0-9][0-9]?)$" // IPv4
	                          ) )
	{
		setupDefaultReplyBuilder();
	}

	/*! Default destructor */
	~Manager() override = default;

	/*! \return Whether the message body of requests is used to match requests.
	 * \sa setInspectBody()
	 */
	bool inspectBody() const override
	{
		return m_inspectBody;
	}

	/*! Defines whether the message body of requests should be used to match requests.
	 * By default, the Manager reads the complete request body into memory to match it against the Rules.
	 * Setting \p inspectBody to \c false prevents that the request body is read into memory.
	 * However, the matching is then done using a null QByteArray() as request body. So Rules with body predicates will
	 * not match unless they match an empty body.
	 * \param inspectBody If \c true (the default), the request body will be read and matched against the predicates of
	 * the Rules. If \c false, the request body will not be read by the Manager but a null QByteArray() will be used
	 * instead.
	 */
	void setInspectBody( bool inspectBody )
	{
		m_inspectBody = inspectBody;
	}

	/*! \return The behavior flags active on this Manager.
	 */
	BehaviorFlags behaviorFlags() const override
	{
		return m_behaviorFlags;
	}

	/*! Tunes the behavior of this Manager.
	 *
	 * \param behaviorFlags Combination of BehaviorFlags to define some details of this Manager's behavior.
	 * \sa BehaviorFlag
	 */
	void setBehaviorFlags( BehaviorFlags behaviorFlags )
	{
		m_behaviorFlags = behaviorFlags;
	}

	/*! \return The maximum number of simultaneous connections to the same target (host + port combination).
	 * \sa setMaxParallelConnectionsPerTarget()
	 */
	unsigned int maxParallelConnectionsPerTarget() const
	{
		return m_connectionPool.maxParallelConnectionsPerTarget();
	}

	/*! Sets the maximum number of simultaneous connections to the same target (host + port combination).
	 *
	 * The default value for this setting is 6. In a real QNetworkAccessManager, this is hard-coded and cannot
	 * be changed.
	 */
	void setMaxParallelConnectionsPerTarget( unsigned int maxParallelConnectionsPerTarget )
	{
		m_connectionPool.setMaxParallelConnectionsPerTarget( maxParallelConnectionsPerTarget );
	}

	/*! \return The time in milliseconds after which an inactive connection is closed.
	 * \sa setConnectionTimeout()
	 */
	unsigned int connectionTimeout() const
	{
		return m_connectionPool.connectionTimeout();
	}

	/*! Sets a timeout after which an inactive connection is closed.
	 *
	 * The default is 0 meaning the connections do not time out. This is also the behavior of a QNetworkAccessManager.
	 *
	 * \param connectionTimeoutInMilliSeconds Timeout in milliseconds. 0 means the connections do not time out.
	 */
	void setConnectionTimeout( unsigned int connectionTimeoutInMilliSeconds )
	{
		m_connectionPool.setConnectionTimeout( connectionTimeoutInMilliSeconds );
	}

	/*! Returns the user defined attributes registered on this Manager.
	 *
	 * See registerUserDefinedAttribute() for details.
	 * \return The set of user defined attributes registered on this Manager.
	 * \sa registerUserDefinedAttribute()
	 * \sa setUserDefinedAttributes()
	 * \since 0.11.0
	 */
	const AttributeSet& userDefinedAttributes() const override
	{
		return m_userDefinedAttributes;
	}

	/*! Sets the set of user defined attributes.
	 *
	 * See registerUserDefinedAttribute() for details.
	 *
	 * \param userDefinedAttributes The set of user defined attributes.
	 * \sa registerUserDefinedAttribute()
	 * \sa userDefinedAttributes()
	 * \since 0.11.0
	 */
	void setUserDefinedAttributes( const AttributeSet& userDefinedAttributes )
	{
		m_userDefinedAttributes = userDefinedAttributes;
	}

	/*! Registers a user defined [QNetworkRequest::Attribute] with this Manager.
	 *
	 * Use this method if the Manager forwards requests to a custom QNetworkAccessManager which sets user defined
	 * attributes on the QNetworkReplies to ensure that the Manager copies those attributes when necessary.
	 *
	 * \param attributeId The ID of the user defined attribute to be made known to this Manager.
	 *
	 * \sa userDefinedAttributes()
	 * \sa setUserDefinedAttributes()
	 * \sa MockReply::clone(const QNetworkReply*, const AttributeSet&)
	 * \since 0.11.0
	 * [QNetworkRequest::Attribute]: http://doc.qt.io/qt-5/qnetworkrequest.html#Attribute-enum
	 */
	void registerUserDefinedAttribute( int attributeId )
	{
		m_userDefinedAttributes.insert( static_cast< QNetworkRequest::Attribute >( attributeId ) );
	}

	/*! Defines how the Manager handles requests that do not match any Rule.
	 *
	 * \param unmatchedRequestBehavior An UnmatchedRequestBehavior flag to define the new behavior.
	 *
	 * \sa unmatchedRequestBehavior()
	 */
	void setUnmatchedRequestBehavior( UnmatchedRequestBehavior unmatchedRequestBehavior )
	{
		m_unmatchedRequestBehavior = unmatchedRequestBehavior;
	}

	/*! \return How the Manager handles unmatched requests.
	 *
	 * \sa setUnmatchedRequestBehavior()
	 */
	UnmatchedRequestBehavior unmatchedRequestBehavior() const override
	{
		return m_unmatchedRequestBehavior;
	}

	/*! Defines a reply builder being used to create replies for requests that do not match any Rule in the Manager.
	 *
	 * \note This builder is only used when unmatchedRequestBehavior() is PredefinedReply.
	 *
	 * \param builder The MockReplyBuilder creating the replies for unmatched requests.
	 * \sa setUnmatchedRequestBehavior()
	 */
	void setUnmatchedRequestBuilder( const MockReplyBuilder& builder )
	{
		m_unmatchedRequestBuilder = builder;
	}

	/*! \return The reply builder being used to create replies for requests that do not match any Rule in the Manager.
	 *
	 * \note This builder is only used when unmatchedRequestBehavior() is PredefinedReply.
	 *
	 * \sa setUnmatchedRequestBuilder()
	 * \sa setUnmatchedRequestBehavior()
	 */
	MockReplyBuilder& unmatchedRequestBuilder()
	{
		return m_unmatchedRequestBuilder;
	}

	/*! \overload
	 * \return A const reference to the reply builder being used to create replies for requests that do not match any
	 * Rule in the manager.
	 * \since 0.11.0
	 */
	const MockReplyBuilder& unmatchedRequestBuilder() const override
	{
		return m_unmatchedRequestBuilder;
	}

	/*! Defines the QNetworkAccessManager to be used in case requests should be forwarded.
	 * By default, the \p Base class of this Manager is used.
	 * \param forwardingTargetNam The network access manager to which the requests are forwarded. If this is a null pointer,
	 * the \p Base class of this Manager is used.
	 * \note This could also be another MockNetworkAccess::Manager. This allows building up a hierarchy of Managers.
	 * \sa setUnmatchedRequestBehavior()
	 * \sa Rule::forward()
	 * \sa \ref page_forwarding
	 */
	void setForwardingTargetNam( QNetworkAccessManager* forwardingTargetNam )
	{
		m_forwardingTargetNam = forwardingTargetNam;
	}

	/*! \return The network access manager to which requests are forwarded or a \c Q_NULLPTR if the requests are
	 * forwarded to the \p Base class of this Manager.
	 *
	 * \note A rule can override the "forwarding target manager" using Rule::forward().
	 */
	QNetworkAccessManager* forwardingTargetNam() const override
	{
		return m_forwardingTargetNam;
	}

	/*! \return The Rules of this Manager.
	 */
	QVector< Rule::Ptr > rules() const override
	{
		return m_rules;
	}

	/*! Sets the Rules for this Manager.
	 * This will remove all previous Rules.
	 * \param rules the new rules for this Manager.
	 */
	void setRules( const QVector< Rule::Ptr >& rules )
	{
		m_rules = rules;
	}

	/*! Adds a Rule to this Manager.
	 * The rule is appended to the existing list of Rules.
	 * \param rule A QSharedPointer to the Rule to be added to this Manager.
	 */
	void addRule( const Rule::Ptr& rule )
	{
		m_rules.append( rule );
	}

	/*! Creates a clone of a Rule and adds it to this Manager.
	 * The clone of the rule is appended to the existing list of Rules.
	 * \param rule The Rule to be added to this Manager.
	 * \return A reference to the clone.
	 * \sa Rule::clone()
	 */
	Rule& addRule( const Rule& rule )
	{
		Rule::Ptr newRule{ rule.clone().release() };
		m_rules.append( newRule );
		return *newRule;
	}

	/*! Removes all rules configured on this Manager.
	 *
	 * \since 1.0.0
	 */
	void clearRules()
	{
		m_rules.clear();
	}

	/*! Creates and adds a Rule which matches \c GET requests with a URL matching a regular expression.
	 * \param urlRegEx The regular expression matched against the request's URL.
	 * \return A reference to the created Rule.
	 */
	Rule& whenGet( const QRegularExpression& urlRegEx )
	{
		return when( QNetworkAccessManager::GetOperation, urlRegEx );
	}

	/*! Creates and adds a Rule which matches \c GET requests with a given URL.
	 * \param url The URL matched against the request's URL.
	 * \return A reference to the created Rule.
	 */
	Rule& whenGet( const QUrl& url )
	{
		return when( QNetworkAccessManager::GetOperation, url );
	}

	/*! Creates and adds a Rule which matches \c POST requests with a URL matching a regular expression.
	 * \param urlRegEx The regular expression matched against the request's URL.
	 * \return A reference to the created Rule.
	 */
	Rule& whenPost( const QRegularExpression& urlRegEx )
	{
		return when( QNetworkAccessManager::PostOperation, urlRegEx );
	}

	/*! Creates and adds a Rule which matches \c POST requests with a given URL.
	 * \param url The URL matched against the request's URL.
	 * \return A reference to the created Rule.
	 */
	Rule& whenPost( const QUrl& url )
	{
		return when( QNetworkAccessManager::PostOperation, url );
	}

	/*! Creates and adds a Rule which matches \c PUT requests with a URL matching a regular expression.
	 * \param urlRegEx The regular expression matched against the request's URL.
	 * \return A reference to the created Rule.
	 */
	Rule& whenPut( const QRegularExpression& urlRegEx )
	{
		return when( QNetworkAccessManager::PutOperation, urlRegEx );
	}

	/*! Creates and adds a Rule which matches \c PUT requests with a given URL.
	 * \param url The URL matched against the request's URL.
	 * \return A reference to the created Rule.
	 */
	Rule& whenPut( const QUrl& url )
	{
		return when( QNetworkAccessManager::PutOperation, url );
	}

	/*! Creates and adds a Rule which matches \c DELETE requests with a URL matching a regular expression.
	 * \param urlRegEx The regular expression matched against the request's URL.
	 * \return A reference to the created Rule.
	 */
	Rule& whenDelete( const QRegularExpression& urlRegEx )
	{
		return when( QNetworkAccessManager::DeleteOperation, urlRegEx );
	}

	/*! Creates and adds a Rule which matches \c DELETE requests with a given URL.
	 * \param url The URL matched against the request's URL.
	 * \return A reference to the created Rule.
	 */
	Rule& whenDelete( const QUrl& url )
	{
		return when( QNetworkAccessManager::DeleteOperation, url );
	}

	/*! Creates and adds a Rule which matches \c HEAD requests with a URL matching a regular expression.
	 * \param urlRegEx The regular expression matched against the request's URL.
	 * \return A reference to the created Rule.
	 */
	Rule& whenHead( const QRegularExpression& urlRegEx )
	{
		return when( QNetworkAccessManager::HeadOperation, urlRegEx );
	}

	/*! Creates and adds a Rule which matches \c HEAD requests with a given URL.
	 * \param url The URL matched against the request's URL.
	 * \return A reference to the created Rule.
	 */
	Rule& whenHead( const QUrl& url )
	{
		return when( QNetworkAccessManager::HeadOperation, url );
	}

	/*! Creates and adds a Rule which matches requests with a given HTTP verb and a URL matching a regular expression.
	 * \param operation The HTTP verb which the request needs to match.
	 * \param urlRegEx The regular expression matched against the request's URL.
	 * \param customVerb The HTTP verb in case \p operation is QNetworkAccessManager::CustomOperation. Else this
	 * parameter is ignored.
	 * \return A reference to the created Rule.
	 */
	Rule& when( QNetworkAccessManager::Operation operation,
	            const QRegularExpression& urlRegEx,
	            const QByteArray& customVerb = QByteArray() )
	{
		using namespace Predicates;
		Rule::Ptr rule( new Rule() );
		rule->has( Verb( operation, customVerb ) );
		rule->has( UrlMatching( urlRegEx ) );
		m_rules.append( rule );
		return *rule;
	}

	/*! Creates and adds a Rule which matches requests with a given HTTP verb and a given URL.
	 * \param operation The HTTP verb which the request needs to match.
	 * \param url The URL matched against the request's URL.
	 * \param customVerb The HTTP verb in case \p operation is QNetworkAccessManager::CustomOperation. Else this
	 * parameter is ignored.
	 * \return A reference to the created Rule.
	 */
	Rule& when( QNetworkAccessManager::Operation operation, const QUrl& url, const QByteArray& customVerb = QByteArray() )
	{
		using namespace Predicates;
		Rule::Ptr rule( new Rule() );
		rule->has( Verb( operation, customVerb ) );
		rule->has( Url( url ) );
		m_rules.append( rule );
		return *rule;
	}

	/*! Provides access to signals of the Manager.
	 *
	 * \return A SignalEmitter object which emits signals on behalf of the Manager.
	 * The ownership of the SignalEmitter stays with the Manager. The caller must not delete it.
	 *
	 * \sa SignalEmitter
	 */
	SignalEmitter* signalEmitter() const
	{
		if ( ! m_signalEmitter )
			m_signalEmitter.reset( new SignalEmitter() );
		return m_signalEmitter.get();
	}

	/*! \return A vector of all requests which this Manager received through its public interface.
	 */
	QVector< Request > receivedRequests() const
	{
		return m_receivedRequests;
	}

	/*! Returns all requests which were handled by this Manager.
	 *
	 * This includes the requests received through the public interface (see receivedRequests()) as well as requests
	 * created internally by the Manager for example when automatically following redirects or when handling
	 * authentication.
	 *
	 * \return A vector of all requests handled by this Manager.
	 */
	QVector< Request > handledRequests() const
	{
		return m_handledRequests;
	}

	/*! \return A vector of all requests which matched a Rule.
	 */
	QVector< Request > matchedRequests() const
	{
		return m_matchedRequests;
	}

	/*! \return A vector of all requests which did not match any Rule.
	 */
	QVector< Request > unmatchedRequests() const
	{
		return m_unmatchedRequests;
	}

	/*! \return A vector of all requests which where forwarded to another network access manager.
	 * \sa setForwardingTargetNam()
	 */
	QVector< Request > forwardedRequests() const
	{
		return m_forwardedRequests;
	}

	/*! Clears all recorded requests.
	 *
	 * \note This also clears all matched requests on the current rules of this Manager.
	 *
	 * \since 1.0.0
	 * \sa receivedRequests()
	 * \sa handledRequests()
	 * \sa matchedRequests()
	 * \sa unmatchedRequests()
	 * \sa forwardedRequests()
	 */
	void clearRequests()
	{
		m_receivedRequests.clear();
		m_handledRequests.clear();
		m_matchedRequests.clear();
		m_unmatchedRequests.clear();
		m_forwardedRequests.clear();

		for ( auto&& rule : detail::asConst( m_rules ) )
			rule->clearMatchedRequests();
	}

	/*! Clears the authentication cache.
	 *
	 * \since 1.0.0
	 * \sa [QNetworkRequest::AuthenticationReuseAttribute](http://doc.qt.io/qt-5/qnetworkrequest.html#Attribute-enum)
	 */
	void clearAuthenticationCache()
	{
		m_authenticationCache.clear();
	}

	/*! Removes all unused connections.
	 *
	 * The Manager simulates keeping connections alive like a real QNetworkAccessManger does when communicating with
	 * an HTTP 1.1 server with a `Connection: keep-alive` header. This method allows "closing" connections which are
	 * not currently in use.
	 *
	 * \note Only unused connections are removed. Connections which are still in use by unfinished replies are not removed.
	 *
	 * \since 1.0.0
	 */
	void clearUnusedConnections()
	{
		m_connectionPool.releaseUnusedConnections();
	}

	/*! Resets the Manager's "runtime state".
	 *
	 * The things being reset are:
	 * - all recorded requests
	 * - authentication cache
	 * - all unused connections
	 *
	 * \note Only unused connections are removed. Connections which are still in use by unfinished replies
	 * are not removed.
	 *
	 * \since 1.0.0
	 * \sa clearRequests()
	 * \sa clearAuthenticationCache()
	 * \sa clearUnusedConnections()
	 */
	void resetRuntimeState()
	{
		clearRequests();
		clearAuthenticationCache();
		clearUnusedConnections();
	}

protected:
	/*! Implements the creation of mocked replies.
	 *
	 * \param operation The HTTP verb of the operation.
	 * \param origRequest The QNetworkRequest object.
	 * \param body Optional request body.
	 * \return A pointer to a QNetworkReply object. The caller takes ownership of the returned reply object. The reply
	 * can either be a real QNetworkReply or a mocked reply. In case of a mocked reply, it is an instance of MockReply.
	 *
	 * \sa QNetworkAccessManager::createRequest()
	 */
	virtual QNetworkReply* createRequest( QNetworkAccessManager::Operation operation,
	                                      const QNetworkRequest& origRequest,
	                                      QIODevice* body ) override;


private:
	const QNetworkAccessManager* publicInterface() const override
	{
		return this;
	}

	QNetworkReply* createRequestBase( QNetworkAccessManager::Operation operation,
	                                  const QNetworkRequest& origRequest,
	                                  QIODevice* body ) override
	{
		return Base::createRequest( operation, origRequest, body );
	}

	detail::ReplyHeaderHandler* replyHeaderHandler() const override
	{
		return m_replyHeaderHandler.get();
	}

	void setupDefaultReplyBuilder()
	{
		m_unmatchedRequestBuilder.withError(
		    QNetworkReply::ContentNotFoundError,
		    QStringLiteral( "MockNetworkAccessManager: Request did not match any rule" ) );
	}

	detail::ConnectionPool& getConnectionPool() override
	{
		return m_connectionPool;
	}


	QNetworkRequest prepareRequest( const QNetworkRequest& origRequest );
	std::unique_ptr< QNetworkReply > handleRequest( const Request& request );
	QAuthenticator getAuthenticator( QNetworkReply* unauthedReply,
	                                 const Request& unauthedReq,
	                                 const HttpUtils::Authentication::Challenge::Ptr& authChallenge ) override;
#if QT_VERSION >= QT_VERSION_CHECK( 5,9,0 )
	void initializeHstsHash();
	void updateHstsHash();
	bool elevateHstsUrl( const QUrl& url );
	bool checkHstsPolicyForHost( const QString& host );
#endif // Qt >= 5.9.0
	std::unique_ptr< QNetworkReply > createDataUrlReply( const Request& request );
	void configureReplyCleanup( QNetworkReply* reply, const Request& request );
	void addReceivedRequest( const Request& request );
	void addHandledRequest( const Request& request ) override;
	void addMatchedRequest( const Request& request, const Rule::Ptr& matchedRule ) override;
	void addUnmatchedRequest( const Request& request ) override;
	void addForwardedRequest( const Request& request ) override;

	bool m_inspectBody;
	BehaviorFlags m_behaviorFlags;
	QPointer< QNetworkAccessManager > m_forwardingTargetNam;
	QVector< Rule::Ptr > m_rules;
	QVector< Request > m_receivedRequests;
	QVector< Request > m_handledRequests;
	QVector< Request > m_matchedRequests;
	QVector< Request > m_unmatchedRequests;
	QVector< Request > m_forwardedRequests;
	mutable std::unique_ptr< SignalEmitter > m_signalEmitter;
	UnmatchedRequestBehavior m_unmatchedRequestBehavior;
	MockReplyBuilder m_unmatchedRequestBuilder;
	std::unique_ptr< detail::ReplyHeaderHandler > m_replyHeaderHandler;
	QHash< QString, QAuthenticator > m_authenticationCache;
#if ( QT_VERSION >= QT_VERSION_CHECK( 5,9,0 ) )
	std::unique_ptr< QHash< QString, QHstsPolicy > > m_hstsHash;
#endif // Qt >= 5.9.0
	QRegularExpression m_ipAddressRegEx;
	AttributeSet m_userDefinedAttributes;
	detail::ConnectionPool m_connectionPool;
};


// ####### Implementation #######

#if defined( MOCKNETWORKACCESSMANAGER_QT_HAS_TEXTCODEC )
inline StringDecoder::StringDecoder( QTextCodec* codec )
    : m_impl( new TextCodecImpl( codec ) )
{
}
#endif

#if QT_VERSION >= QT_VERSION_CHECK( 6,0,0 )
inline StringDecoder::StringDecoder( std::unique_ptr< QStringDecoder >&& decoder )
    : m_impl{ new StringDecoderImpl( std::move( decoder ) ) }
{
}
#endif

/*! \internal Implementation details
 */
namespace detail {

	inline bool requestLoadsCookies( const QNetworkRequest& request )
	{
		const auto defaultValue = QVariant::fromValue( static_cast< int >( QNetworkRequest::Automatic ) );
		const auto requestValue = request.attribute( QNetworkRequest::CookieLoadControlAttribute, defaultValue ).toInt();
		return static_cast< QNetworkRequest::LoadControl >( requestValue ) == QNetworkRequest::Automatic;
	}

} // namespace detail

template< class Matcher >
Rule& Rule::isMatching( const Matcher& matcher )
{
	m_predicates.append( Predicates::createGeneric( matcher ) );
	return *this;
}

template< class Matcher >
Rule& Rule::isNotMatching( const Matcher& matcher )
{
	Predicate::Ptr predicate = Predicates::createGeneric( matcher );
	predicate->negate();
	m_predicates.append( predicate );
	return *this;
}

template< class Base >
QNetworkReply* Manager< Base >::createRequest( QNetworkAccessManager::Operation operation,
                                               const QNetworkRequest& origRequest,
                                               QIODevice* body )
{
	QByteArray data;
	if ( m_inspectBody && body )
		data = body->readAll();
	const QNetworkRequest preparedRequest = prepareRequest( origRequest );
	const Request request( operation, preparedRequest, data );

	addReceivedRequest( request );
	auto reply = handleRequest( request );
	configureReplyCleanup( reply.get(), request );
	return reply.release();
}

template< class Base >
QNetworkRequest Manager< Base >::prepareRequest( const QNetworkRequest& origRequest )
{
	QNetworkRequest request( origRequest );

#if ( QT_VERSION >= QT_VERSION_CHECK( 5,9,0 ) )
	if ( this->isStrictTransportSecurityEnabled() && elevateHstsUrl( request.url() ) )
	{
		QUrl url = request.url();
		url.setScheme( HttpUtils::httpsScheme() );
		if ( url.port() == HttpUtils::HttpDefaultPort )
			url.setPort( HttpUtils::HttpsDefaultPort );
		request.setUrl( url );
	}
#endif // Qt >= 5.9.0

	const bool loadCookies = detail::requestLoadsCookies( request );
	if ( loadCookies )
	{
		QNetworkCookieJar* cookieJar = this->cookieJar();
		if ( cookieJar )
		{
			QUrl requestUrl = request.url();
			if ( requestUrl.path().isEmpty() )
				requestUrl.setPath( QStringLiteral( "/" ) );
			QList< QNetworkCookie > cookies = cookieJar->cookiesForUrl( requestUrl );
			if ( ! cookies.isEmpty() )
				request.setHeader( QNetworkRequest::CookieHeader, QVariant::fromValue( cookies ) );
		}
	}

	return request;
}

template< class Base >
void Manager< Base >::addReceivedRequest( const Request& request )
{
	m_receivedRequests.append( request );
	if ( m_signalEmitter )
		Q_EMIT m_signalEmitter->receivedRequest( request );
}

template< class Base >
std::unique_ptr< QNetworkReply > Manager< Base >::handleRequest( const Request& request )
{
	if ( detail::isDataUrlRequest( request ) )
	{
		addHandledRequest( request );
		return createDataUrlReply( request );
	}

	std::unique_ptr< NetworkReply > reply{ new NetworkReply{ this } };
	reply->setBehaviorFlags( m_behaviorFlags );
	reply->setUserDefinedAttributes( m_userDefinedAttributes );
	reply->run( this, request );
	return reply;
}

template< class Base >
void Manager< Base >::addHandledRequest( const Request& request )
{
	m_handledRequests.append( request );
	if ( m_signalEmitter )
		Q_EMIT m_signalEmitter->handledRequest( request );
}

template< class Base >
void Manager< Base >::addMatchedRequest( const Request& request, const Rule::Ptr& matchedRule )
{
	m_matchedRequests.append( request );
	matchedRule->m_matchedRequests.append( request );
	if ( m_signalEmitter )
		Q_EMIT m_signalEmitter->matchedRequest( request, matchedRule );
}

template< class Base >
void Manager< Base >::addUnmatchedRequest( const Request& request )
{
	m_unmatchedRequests.append( request );
	if ( m_signalEmitter )
		Q_EMIT m_signalEmitter->unmatchedRequest( request );
}

#if ( QT_VERSION >= QT_VERSION_CHECK( 5,9,0 ) )

template< class Base >
void Manager< Base >::initializeHstsHash()
{
	if ( ! m_hstsHash )
	{
		m_hstsHash.reset( new QHash< QString, QHstsPolicy >() );
		updateHstsHash();
	}
}

template< class Base >
void Manager< Base >::updateHstsHash()
{
	const auto hstsPolicies = this->strictTransportSecurityHosts();

	m_hstsHash->clear();

	auto policyIter = hstsPolicies.cbegin();
	const auto policyEnd = hstsPolicies.cend();
	for ( ; policyIter != policyEnd; ++policyIter )
	{
		if ( ! policyIter->isExpired() )
			m_hstsHash->insert( policyIter->host(), *policyIter );
	}
}

template< class Base >
bool Manager< Base >::elevateHstsUrl( const QUrl& url )
{
	if ( ! url.isValid() || url.scheme().toLower() != HttpUtils::httpScheme() )
		return false;

	const auto host = url.host();
	if ( m_ipAddressRegEx.match( host ).hasMatch() )
		return false; // Don't elevate IP address URLs

	initializeHstsHash();

	bool retryAfterUpdate = false;

	do
	{
		if ( checkHstsPolicyForHost( host ) )
			return true;

		if ( retryAfterUpdate )
			break;
		updateHstsHash();
		retryAfterUpdate = true;
	} while ( true );

	return false;
}

template< class Base >
bool Manager< Base >::checkHstsPolicyForHost( const QString& host )
{
	// Check if there is a policy for the full host name
	auto hstsHashIter = m_hstsHash->find( host );

	if ( hstsHashIter != m_hstsHash->end() )
	{
		if ( ! hstsHashIter.value().isExpired() )
			return true;
		hstsHashIter = m_hstsHash->erase( hstsHashIter );
	}

	// Check if there is a policy for a parent domain
	auto domainParts = host.split( QChar::fromLatin1( '.' ), Qt::SplitBehaviorFlags::SkipEmptyParts );
	domainParts.pop_front();

	while ( ! domainParts.isEmpty() )
	{
		hstsHashIter = m_hstsHash->find( domainParts.join( QChar::fromLatin1( '.' ) ) );
		if ( hstsHashIter != m_hstsHash->end() )
		{
			if ( hstsHashIter.value().isExpired() )
				hstsHashIter = m_hstsHash->erase( hstsHashIter );
			else if ( hstsHashIter.value().includesSubDomains() )
				return true;
			// else we continue because there could be a policy for a another parent domain that includes sub domains
		}
		domainParts.pop_front();
	}

	return false;
}

#endif // Qt >= 5.9.0

template< class Base >
std::unique_ptr< QNetworkReply > Manager< Base >::createDataUrlReply( const Request& request )
{
	std::unique_ptr< QNetworkReply > reply{ Base::createRequest( request.operation, request.qRequest, nullptr ) };
	return reply;
}


template< class Base >
void Manager< Base >::configureReplyCleanup( QNetworkReply* reply, const Request& request )
{
	reply->setParent( this );
	QObject::connect( reply, &QNetworkReply::finished, this, [ reply, this ]() {
		if ( reply->parent() == this )
		{
			reply->setParent( nullptr );
		}
	} );

#if QT_VERSION >= QT_VERSION_CHECK( 5,14,0 )
	if ( this->autoDeleteReplies()
	     || request.qRequest.attribute( QNetworkRequest::AutoDeleteReplyOnFinishAttribute ).toBool() )
	{
		QObject::connect( reply, &QNetworkReply::finished, reply, &QObject::deleteLater );
	}
#else  // Qt < 5.14.0
	Q_UNUSED( request )
#endif // Qt < 5.14.0
}

template< class Base >
void Manager< Base >::addForwardedRequest( const Request& request )
{
	m_forwardedRequests.append( request );
	if ( m_signalEmitter )
		Q_EMIT m_signalEmitter->forwardedRequest( request );
}

template< class Base >
QAuthenticator Manager< Base >::getAuthenticator( QNetworkReply* unauthedReply,
                                                  const Request& unauthedReq,
                                                  const HttpUtils::Authentication::Challenge::Ptr& authChallenge )
{
	const auto realm = authChallenge->realm().toLower(); // realm is case-insensitive
	const auto authScope = HttpUtils::Authentication::authenticationScopeForUrl( unauthedReply->url() );
	const auto authKey = realm + QChar::fromLatin1( '\x1C' ) + authScope.toString( QUrl::FullyEncoded );
	const auto authReuse = static_cast< QNetworkRequest::LoadControl >(
	    unauthedReq.qRequest
	        .attribute( QNetworkRequest::AuthenticationReuseAttribute, static_cast< int >( QNetworkRequest::Automatic ) )
	        .toInt() );

	if ( authReuse == QNetworkRequest::Automatic && m_authenticationCache.contains( authKey ) )
		return m_authenticationCache.value( authKey );
	else
	{
		QAuthenticator authenticator;
		authenticator.setOption( HttpUtils::Authentication::Challenge::realmKey(), realm );
		authenticator.setRealm( realm );
		Q_EMIT this->authenticationRequired( unauthedReply, &authenticator );
		if ( ! authenticator.user().isNull() || ! authenticator.password().isNull() )
			m_authenticationCache.insert( authKey, authenticator );
		return authenticator;
	}
}


} // namespace MockNetworkAccess

Q_DECLARE_METATYPE( MockNetworkAccess::MockReplyBuilder )
Q_DECLARE_METATYPE( MockNetworkAccess::HttpStatus::Code )
Q_DECLARE_OPERATORS_FOR_FLAGS( MockNetworkAccess::BehaviorFlags )
Q_DECLARE_METATYPE( MockNetworkAccess::RequestList )


#endif /* MOCKNETWORKACCESSMANAGER_HPP */
