open ReasonApolloTypes;

module Get = (Config: ReasonApolloTypes.Config) => {
  [@bs.module] external gql : ReasonApolloTypes.gql = "graphql-tag";
  [@bs.module "react-apollo"]
  external queryComponent : ReasonReact.reactClass = "Query";
  type response =
    | Loading
    | Error(apolloError)
    | Data(Config.t)
    | NoData;
  type renderPropObj = {
    result: response,
    data: option(Config.t),
    error: option(apolloError),
    loading: bool,
    refetch: option(Js.Json.t) => Js.Promise.t(response),
    fetchMore: (~variables: Js.Json.t) => Js.Promise.t(unit),
    networkStatus: int,
  };
  type renderPropObjJS = {
    .
    "loading": bool,
    "data": Js.Nullable.t(Js.Json.t),
    "error": Js.Nullable.t(apolloError),
    "refetch":
      [@bs.meth] (
        Js.Null_undefined.t(Js.Json.t) => Js.Promise.t(renderPropObjJS)
      ),
    "networkStatus": int,
    "variables": Js.Null_undefined.t(Js.Json.t),
    "fetchMore": [@bs.meth] (apolloOptions => Js.Promise.t(unit)),
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
  type queryWithData = queryObjWithData(Config.t);
  [@bs.send]
  external setData : (proxy, queryWithData) => unit =
    "writeQuery";
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
        apolloData##loading,
        apolloData##data |> Js.Nullable.toOption,
        apolloData##error |> Js.Nullable.toOption,
      ) {
      | (true, _, _) => Loading
      | (false, Some(response), _) => Data(Config.parse(response))
      | (false, _, Some(error)) => Error(error)
      | (false, None, None) => NoData
      };
  let convertJsInputToReason = (apolloData: renderPropObjJS) => {
    result: apolloData |> apolloDataToVariant,
    data:
      switch (apolloData##data |> ReasonApolloUtils.getNonEmptyObj) {
      | None => None
      | Some(data) =>
        switch (Config.parse(data)) {
        | parsedData => Some(parsedData)
        | exception _ => None
        }
      },
    error:
      switch (apolloData##error |> Js.Nullable.toOption) {
      | Some(error) => Some(error)
      | None => None
      },
    loading: apolloData##loading,
    refetch: variables =>
      apolloData##refetch(variables |> Js.Nullable.fromOption)
      |> Js.Promise.then_(data =>
           data |> apolloDataToVariant |> Js.Promise.resolve
         ),
    fetchMore: (~variables) =>
      apolloData##fetchMore({
        "variables": variables,
        "query": graphqlQueryAST,
      }),
    networkStatus: apolloData##networkStatus,
  };
  let make =
      (
        ~variables: option(Js.Json.t)=?,
        ~pollInterval: option(int)=?,
        ~notifyOnNetworkStatusChange: option(bool)=?,
        ~fetchPolicy: option(string)=?,
        ~errorPolicy: option(string)=?,
        ~ssr: option(bool)=?,
        ~displayName: option(string)=?,
        ~delay: option(bool)=?,
        ~context: option(Js.Json.t)=?,
        children: renderPropObj => ReasonReact.reactElement,
      ) =>
    ReasonReact.wrapJsForReason(
      ~reactClass=queryComponent,
      ~props=
        Js.Nullable.(
          {
            "query": graphqlQueryAST,
            "variables": variables |> fromOption,
            "pollInterval": pollInterval |> fromOption,
            "notifyOnNetworkStatusChange":
              notifyOnNetworkStatusChange |> fromOption,
            "fetchPolicy": fetchPolicy |> fromOption,
            "errorPolicy": errorPolicy |> fromOption,
            "ssr": ssr |> fromOption,
            "displayName": displayName |> fromOption,
            "delay": delay |> fromOption,
            "context": context |> fromOption,
          }
        ),
      apolloData =>
      apolloData |> convertJsInputToReason |> children
    );
};
