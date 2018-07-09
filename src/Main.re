open Github;

[@bs.val] external scrollTo : (int, int) => unit = "";

module ErrorMessage = {
    let component = ReasonReact.statelessComponent("ErrorMessage");
    let make = (~value, ~onClose=(_=>()), children) => ReasonReact.({
        ...component,
        render: _self =>
            <div className="ui error message">
                <i className="close icon" onClick=onClose />
                <div className="header">
                    (string("There was an error when communicating with the API"))
                </div>
                /* FIXME: Can we do better tah n stringifyAny? */
                <p>(string(Js.Option.default("", Js.Json.stringifyAny(value))))</p>
                (array(children))
            </div>
    });
};

type action =
  | Init
  | GetRepositories(bool)
  | GotRepositories(bool, fetchResponse(array(repository)))
  | GetStargazers
  | GotStargazers(fetchResponse(array(stargazer)))
  | SetSelected(repository)
  | SetOrgname(string)
  | ClearLastError
  ;

type state = {
    orgname: string,
    repositories: option(array(repository)),
    stargazers: option(array(stargazer)),
    selectedRepo: option(repository),
    nextPageUrl: option(string),
    lastError: option(Js.Promise.error),
    loadingRepos: bool,
    loadingStargazers: bool,
    stargazersPanelRef: ref(option(Dom.element))
};


let mkReducer = (~client=defaultClient, ~maxStargazers=20, ()) => action => {
    let getRepositories = self => self . ReasonReact.send(GetRepositories(false));
    switch (action) {
    | Init => (_state => ReasonReact.SideEffects(getRepositories));
    
    | GetRepositories(append) => (state =>
        ReasonReact.UpdateWithSideEffects({...state, lastError:None, loadingRepos:true}, self => {
            if (String.length(state.orgname) > 0) {
                let url = Js.Option.getWithDefault (orgReposUrl(state.orgname)) (state.nextPageUrl);
                Js.Promise.(
                    client.fetchOrgRepositories(url)
                    |> then_(result => self.send(GotRepositories(append, result)) |> resolve)
                    |> ignore
                )
            }
        }))

    | GotRepositories(append, result) => (state =>
        switch(result) {
        | Right((nextPageUrl,repos)) => {
            let repositories =
                if (append) {
                    switch(state.repositories) {
                    | Some(curRepos) => Some(Array.append(curRepos, repos));
                    | None           => Some(repos);
                    }
                } else Some(repos);
            ReasonReact.Update({...state, loadingRepos:false, repositories, nextPageUrl});
            }
        | Left(err) => ReasonReact.Update({...state, loadingRepos:false, lastError: Some(err)});
        });

    | GetStargazers => (state =>
        switch(state.selectedRepo) {
        | Some(repo) =>
            ReasonReact.UpdateWithSideEffects({...state, lastError:None, loadingStargazers:true}, self => {
                Js.Promise.(
                    fetchStargazers(repo.stargazerUrl)
                    |> then_(result => self.send(GotStargazers(result)) |> resolve)
                    |> ignore
                )
            })
        | None => ReasonReact.NoUpdate
        })

    | GotStargazers(result) => (state =>
        switch(result) {
        | Right((_,stargazers)) => {
            let count = Js.Math.min_int(maxStargazers, Array.length(stargazers));
            let limitedStargazers = Array.sub(stargazers, 0, count);
            ReasonReact.UpdateWithSideEffects(
                { ...state
                , loadingStargazers:false
                , stargazers: Some(limitedStargazers)
                }, (_self =>
                    switch(state.stargazersPanelRef^) {
                    | Some(r) => scrollTo(0, ReactDOMRe.domElementToObj(r)##offsetTop)
                    | None    => ()
                    })
                );
            }
        | Left(err) => ReasonReact.Update({...state, loadingStargazers:false, lastError: Some(err)});
        });

    | SetOrgname(orgname) => (state =>
        ReasonReact.UpdateWithSideEffects({...state, orgname, nextPageUrl:None}, getRepositories));

    | SetSelected(repo) => (state =>
        ReasonReact.UpdateWithSideEffects(
            {...state, selectedRepo:Some(repo)},
            (self => self.send(GetStargazers)))
        );
    | ClearLastError => (state => ReasonReact.Update({...state, lastError:None}))
    }
}

let component = ReasonReact.reducerComponent("Main");

let make = (~orgname, ~client=defaultClient, ~maxStargazers=20, _children) => {
    ...component,

    didMount: self => self.send(Init),

    initialState: () => {
        orgname,
        repositories: None,
        stargazers: None,
        selectedRepo: None,
        nextPageUrl: None,
        lastError: None,
        loadingRepos: false,
        loadingStargazers: false,
        stargazersPanelRef: ref(None)

    },

    reducer: mkReducer(~client, ~maxStargazers, ()),

    render: ({state,send,handle}) => ReasonReact.({
        let reposColumn =
            <div className="repositories column">
                <h2 className="header">
                    (string(state.orgname ++ "'s repositories"))
                </h2>
                (switch(state.repositories) {
                | Some(repositories) =>
                    <Repositories repositories
                        onStargazersClick=(repo =>
                            send(SetSelected(repo)))
                        />
                | None => null
                })
                (switch (state.nextPageUrl) {
                | Some(_) =>
                    <a href="#"
                       className="link more"
                        onClick=(ev => {
                            ReactEventRe.Mouse.preventDefault(ev);
                            send(GetRepositories(true));
                        })>(string("Load more"))</a>
                | None => null
                })
            </div>;
        let setStargazersPanelRef = handle((ref,{state}) =>
            state.stargazersPanelRef := Js.Nullable.toOption(ref));
        let stargazersColumn =
            switch(state.selectedRepo) {
            | Some(repo) =>
                <div className="stargazers column" ref=setStargazersPanelRef>
                    <h2 className="header">
                        (string(repo.fullName ++ "'s stargazers"))
                    </h2>
                    <Stargazers stargazers=state.stargazers />
                </div>
            | None => <div className="column" />
            };

        <div className="ui container">
            <TextInput
                value=state.orgname
                onChange=(orgname => (send(SetOrgname(orgname))))
                />
            (switch(state.lastError) {
            | Some(value) =>
                <ErrorMessage value onClose=(_=> send(ClearLastError)) />
            | None => ReasonReact.null
            })
            <div className="ui segment">
                <div className="ui two column stackable grid layout">
                    reposColumn
                    stargazersColumn
                </div>
            </div>
        </div>
    })

}