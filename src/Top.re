open Github;

[@bs.val] external scrollTo : (int, int) => unit = "";

type action =
  | Init
  | GetRepositories(bool)
  | GotRepositories(bool, fetchResponse(array(repository)))
  | SetSelected(repository)
  | SetOrgname(string)
  ;

type state = {
    orgname: string,
    repositories: option(array(repository)),
    selectedRepo: option(repository),
    nextPageUrl: option(string),
    lastError: option(Js.Promise.error),
    loading: bool,
    stargazersPanelRef: ref(option(Dom.element))
};

type client = {
    fetchOrgRepositories: getRepositories,
    fetchStargazers: getStargazers
}

let defaultClient = {
    fetchOrgRepositories: fetchOrgRepositories,
    fetchStargazers: fetchStargazers
};

let component = ReasonReact.reducerComponent("ReGitHubStargazers");

let make = (~orgname, ~client=defaultClient, _children) => {
    ...component,

    initialState: () => {
        orgname,
        repositories: None,
        selectedRepo: None,
        nextPageUrl: None,
        lastError: None,
        loading: false,
        stargazersPanelRef: ref(None)

    },

    didMount: self => self.send(Init),

    reducer: action => {
        let getRepositories = self => self . ReasonReact.send(GetRepositories(false));
        switch (action) {
        | Init => (_state => ReasonReact.SideEffects(getRepositories));
        
        | GetRepositories(append) => (state =>
            ReasonReact.UpdateWithSideEffects({...state, lastError:None, loading:true}, self => {
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
                ReasonReact.Update({...state, loading:false, repositories, nextPageUrl});
                }
            | Left(err) => ReasonReact.Update({...state, loading:false, lastError: Some(err)});
            });

        | SetOrgname(orgname) => (state =>
            ReasonReact.UpdateWithSideEffects({...state, orgname, nextPageUrl:None}, getRepositories));

        | SetSelected(repo) => (state =>
            ReasonReact.UpdateWithSideEffects({...state, selectedRepo:Some(repo)}, (self =>
                switch(state.stargazersPanelRef^) {
                | Some(r) => scrollTo(0, ReactDOMRe.domElementToObj(r)##offsetTop)
                | None    => ()
                }
            )));
        }
    },

    render: ({state,send,handle}) => {
        open ReasonReact;
        let setStargazersPanelRef = handle((ref,{state}) =>
            state.stargazersPanelRef := Js.Nullable.toOption(ref));
        <div className="ui container">
            <TextInput value=state.orgname onChange=(orgname => (send(SetOrgname(orgname)))) />
            <div className="ui segment">
                <div className="ui two column stackable grid layout">

                    <div className="column">
                        <h2 className="header">(string(state.orgname))</h2>
                        <div className="ui list">
                            (switch(state.repositories) {
                                | Some(repos) => array(Array.map((repository =>
                                    <Repository
                                        repository
                                        onStargazersClick=(ev => {
                                            ReactEventRe.Mouse.preventDefault(ev);
                                            send(SetSelected(repository))
                                        })
                                        />
                                    ), repos))
                                | None => null
                            })
                        </div>
                        (switch (state.nextPageUrl) {
                        | Some(_) =>
                            <a href="#"
                               onClick=(ev => {
                                   ReactEventRe.Mouse.preventDefault(ev);
                                   send(GetRepositories(true));
                               })>(string("Load more"))</a>
                        | None => ReasonReact.null
                        })
                    </div>

                    <div className="column" ref=setStargazersPanelRef>
                        (switch(state.selectedRepo) {
                            | Some(repo) =>
                                <div>
                                    <h2 className="header">(string(repo.fullName ++ "'s stargazers"))</h2>
                                    <Stargazers
                                        url=repo.stargazerUrl
                                        fetchStargazers=client.fetchStargazers
                                        />
                                </div>
                            | None => null
                        })
                    </div>

                </div>
            </div>
        </div>
    }

}