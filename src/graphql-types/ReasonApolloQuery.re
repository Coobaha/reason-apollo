open ReasonApolloTypes;

[@bs.deriving abstract]
type updateQueryOptions = {
  [@bs.optional]
  fetchMoreResult: Js.Json.t,
  [@bs.optional]
  variables: Js.Json.t,
};

type onErrorT;
type updateQueryT = (Js.Json.t, updateQueryOptions) => Js.Json.t;
type updateSubscriptionOptions = {
  .
  "subscriptionData": Js.Nullable.t(Js.Json.t),
  "variables": Js.Nullable.t(Js.Json.t),
};
type updateQuerySubscriptionT =
  (Js.Json.t, updateSubscriptionOptions) => Js.Json.t;

[@bs.deriving abstract]
type subscribeToMoreOptions = {
  document: queryString,
  [@bs.optional]
  variables: Js.Json.t,
  [@bs.optional]
  updateQuery: updateQuerySubscriptionT,
  [@bs.optional]
  onError: onErrorT,
};

/* We dont accept a new query for now */
[@bs.deriving abstract]
type fetchMoreOptions = {
  [@bs.optional]
  variables: Js.Json.t,
  updateQuery: updateQueryT,
};

[@bs.deriving abstract]
type renderPropObjJS = {
  loading: bool,
  data: Js.Nullable.t(Js.Json.t),
  error: Js.Nullable.t(apolloError),
  refetch: Js.Null_undefined.t(Js.Json.t) => Js.Promise.t(renderPropObjJS),
  networkStatus: int,
  variables: Js.Null_undefined.t(Js.Json.t),
  fetchMore: fetchMoreOptions => Js.Promise.t(unit),
  subscribeToMore: (subscribeToMoreOptions, unit) => unit,
};

module Get = (Config: ReasonApolloTypes.Config) => {
  [@bs.module] external gql: ReasonApolloTypes.gql = "graphql-tag";
  [@bs.module "react-apollo"]
  external queryComponent: ReasonReact.reactClass = "Query";
  type response = queryResponse(Config.t);

  type renderPropObj = {
    result: response,
    data: option(Config.t),
    variables: option(Js.Json.t),
    error: option(apolloError),
    loading: bool,
    refetch: option(Js.Json.t) => Js.Promise.t(response),
    fetchMore:
      (~variables: Js.Json.t=?, ~updateQuery: updateQueryT, unit) =>
      Js.Promise.t(unit),
    networkStatus: int,
    subscribeToMore:
      (
        ~document: queryString,
        ~variables: Js.Json.t=?,
        ~updateQuery: updateQuerySubscriptionT=?,
        ~onError: onErrorT=?,
        unit,
        unit
      ) =>
      unit,
  };

  let graphqlQueryAST = gql(. Config.query);
  [@bs.send]
  external getData : (proxy, queryObj) => Js.Nullable.t(Config.t) =
    "readQuery";
  let getData = (~variables: option(Js.Json.t)=?, proxy) =>
    getData(
      proxy,
      {
        "query": graphqlQueryAST,
        "variables": variables |> Js.Nullable.fromOption,
      },
    );
  type queryWithData = queryObjWithData(Js.Nullable.t(Config.t));
  [@bs.send] external setData : (proxy, queryWithData) => unit = "writeQuery";
  let setData = (~variables: option(Js.Json.t)=?, ~data, proxy) =>
    setData(
      proxy,
      {
        "query": graphqlQueryAST,
        "variables": variables |> Js.Nullable.fromOption,
        "data": data,
      },
    );
  let apolloDataToVariant: renderPropObjJS => response =
    apolloData =>
      switch (
        apolloData->loadingGet,
        apolloData->dataGet |> Js.Nullable.toOption,
        apolloData->errorGet |> Js.Nullable.toOption,
      ) {
      | (true, _, _) => Loading
      | (false, Some(response), _) => Data(Config.parse(response))
      | (false, _, Some(error)) => Error(error)
      | (false, None, None) =>
        Error(apolloError(~message="No Data", ~graphQLErrors=[||], ()))
      };

  let convertJsInputToReason = (apolloData: renderPropObjJS) => {
    result: apolloData |> apolloDataToVariant,
    data:
      switch (apolloData->dataGet |> ReasonApolloUtils.getNonEmptyObj) {
      | None => None
      | Some(data) =>
        switch (Config.parse(data)) {
        | parsedData => Some(parsedData)
        | exception _ => None
        }
      },
    error:
      switch (apolloData->errorGet |> Js.Nullable.toOption) {
      | Some(error) => Some(error)
      | None => None
      },
    loading: apolloData->loadingGet,
    variables: apolloData->variablesGet |> Js.Nullable.toOption,
    refetch: variables =>
      apolloData->(refetchGet(variables |> Js.Nullable.fromOption))
      |> Js.Promise.then_(data =>
           data |> apolloDataToVariant |> Js.Promise.resolve
         ),
    fetchMore: (~variables=?, ~updateQuery, ()) =>
      apolloData
      ->(fetchMoreGet(fetchMoreOptions(~variables?, ~updateQuery, ()))),
    networkStatus: apolloData->networkStatusGet,
    subscribeToMore:
      (~document, ~variables=?, ~updateQuery=?, ~onError=?, ()) =>
      apolloData
      ->(
          subscribeToMoreGet(
            subscribeToMoreOptions(
              ~document,
              ~variables?,
              ~updateQuery?,
              ~onError?,
              (),
            ),
          )
        ),
  };

  let make =
      (
        ~variables: option(Js.Json.t)=?,
        ~pollInterval: option(int)=?,
        ~notifyOnNetworkStatusChange: option(bool)=?,
        ~fetchPolicy: option(fetchPolicy)=?,
        ~errorPolicy: option(errorPolicy)=?,
        ~ssr: option(bool)=?,
        ~displayName: option(string)=?,
        ~delay: option(bool)=?,
        ~context: option(Js.Json.t)=?,
        children: renderPropObj => ReasonReact.reactElement,
      ) =>
    ReasonReact.wrapJsForReason(
      ~reactClass=queryComponent,
      ~props=
        Js.Nullable.{
          "query": graphqlQueryAST,
          "variables": variables |> fromOption,
          "pollInterval": pollInterval |> fromOption,
          "notifyOnNetworkStatusChange":
            notifyOnNetworkStatusChange |> fromOption,
          "fetchPolicy":
              switch (fetchPolicy) {
              | Some(policy) =>
                switch (policy) {
                | CacheFirst => return("cache-first")
                | CacheAndNetwork => return("cache-and-network")
                | CacheOnly => return("cache-only")
                | NetworkOnly => return("network-only")
                | Standby => return("standby")
                }
              | None => undefined
              },
          "errorPolicy":
              switch (errorPolicy) {
              | Some(policy) =>
                switch (policy) {
                | None' => return("none")
                | All => return("all")
                | Ignore => return("ignore")
                }
              | None => undefined
              },
          "ssr": ssr |> fromOption,
          "displayName": displayName |> fromOption,
          "delay": delay |> fromOption,
          "context": context |> fromOption,
        },
      apolloData =>
      apolloData |> convertJsInputToReason |> children
    );
};
