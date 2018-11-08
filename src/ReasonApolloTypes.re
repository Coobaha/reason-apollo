/**
 * An abstract type to describe a query string object.
 */
type queryString;

/**
 * The signature of the `graphql-tag/gql` function that transforms a GraphQL
 * query string to the standard GraphQL AST.
 * https://github.com/apollographql/graphql-tag
 */
type gql = (. string) => queryString;

/**
 * An abstract type to describe an Apollo Link object.
 */
type apolloLink;

/**
 * An abstract type to describe an Apollo Cache object.
 */
type apolloCache;

type networkError = {. "statusCode": int};

module Language = {
  type location = {
    .
    "line": int,
    "column": int,
  };
  type source = {
    .
    "body": string,
    "name": string,
    "locationOffset": location,
  };
  type astNode;
};

/***
 * Represents a GraphQL Error type
 */
type graphqlError = {
  .
  "message": string,
  "locations": Js.Nullable.t(array(Language.location)),
  "nodes": Js.Nullable.t(array(Language.astNode)),
  "source": Js.Nullable.t(Language.source),
  "positions": Js.Nullable.t(array(int)),
  "originalError": Js.Nullable.t(Js.Exn.t),
};

[@bs.deriving abstract]
type apolloError = {
  message: string,
  graphQLErrors: array(graphqlError),
  [@bs.optional]
  networkError: Js.Nullable.t(string),
  [@bs.optional] [@bs.as "networkError"]
  networkErrorObj:
    Js.Nullable.t({
      .
      "message": string,
      "result": Js.Nullable.t({. "errors": array(graphqlError)}),
    }),
};

type executionResult = {
  .
  "errors": Js.Nullable.t(Js.Array.t(graphqlError)),
  "data": Js.Nullable.t(Js.Json.t),
};

/* TODO define all types */
type operation = {. "query": queryString};

/* TODO define subscription */
type subscription;

type errorResponse = {
  .
  "graphQLErrors": Js.Nullable.t(Js.Array.t(graphqlError)),
  "networkError": Js.Nullable.t(networkError),
  "response": Js.Nullable.t(executionResult),
  "operation": operation,
  "forward": operation => subscription,
};

module type Config = {
  let query: string;
  type t;
  let parse: Js.Json.t => t;
};

type queryObj = {
  .
  "query": queryString,
  "variables": Js.Null_undefined.t(Js.Json.t),
};

type queryObjWithData('a) = {
  .
  "query": queryString,
  "variables": Js.Null_undefined.t(Js.Json.t),
  "data": 'a,
};

type mutationObj = {
  .
  "mutation": queryString,
  "variables": Js.Null_undefined.t(Js.Json.t),
};

type apolloOptions = {
  .
  "query": queryString,
  "variables": Js.Null_undefined.t(Js.Json.t),
};

type queryResponse('a) =
  | Loading
  | Error(apolloError)
  | Data('a);

type mutationResponse('a) =
  | Loading
  | Error(apolloError)
  | Data('a)
  | NotCalled;

type subscriptionResponse('a) =
  | Loading
  | Error(apolloError)
  | Data('a);

/*cache DataProxy*/
type proxy;

/*
 fetchPolicy determines where the client may return a result from. The options are:
   - cache-first (default): return result from cache. Only fetch from network if cached result is not available.
   - cache-and-network: returns result from cache first (if it exists), then return network result once it's available
   - cache-only: return result from cache if avaiable, fail otherwise.
   - network-only: return result from network, fail if network call doesn't succeed.
   - standby: only for queries that aren't actively watched, but should be available for refetch and updateQueries.
 */
type fetchPolicy =
  | CacheFirst
  | CacheAndNetwork
  | CacheOnly
  | NetworkOnly
  | Standby;

/*
  errorPolicy determines the level of events for errors in the execution result. The options are:
    - None' (default): any errors from the request are treated like runtime errors and the observable is stopped
    - ignore: errors from the request do not stop the observable, but also don't call `next`
    - all: errors are treated like data and will notify observables
 */
type errorPolicy =
  | None'
  | All
  | Ignore;

/*
 apollo link ws
 */

[@bs.deriving abstract]
type webSocketLinkOptionsT = {
  [@bs.optional]
  reconnect: bool,
};

[@bs.deriving abstract]
type webSocketLinkT = {
  uri: string,
  options: webSocketLinkOptionsT,
};

type documentNodeT;

type splitTest = {. "query": documentNodeT};
